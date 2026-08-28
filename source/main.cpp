#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

const int PK9_PARTY  = 344;

struct GameDef {
    u64 tid;
    const char* name;
    const char* tag;
    u32 keyMyStatus, keyMoney, keyBox;
    int boxStride;
};
static const GameDef GAMES[] = {
    { 0x0100A3D008C5C000ULL, "Pokemon Scarlet", "SC", 0xE3E89BD1, 0x4F35D0DD, 0x0d66012c, 344 },
    { 0x01008F6008C5E000ULL, "Pokemon Violet",  "VI", 0xE3E89BD1, 0x4F35D0DD, 0x0d66012c, 344 },
};
static const int GAME_COUNT = 2;
static const GameDef* g_game = &GAMES[0];
static bool g_installed[GAME_COUNT] = {false,false};
static u32 KMyStatus, KMoney, KBox;
static int BOX_STRIDE;

static void detectGames(){
    if (R_FAILED(nsInitialize())) return;
    s32 offset = 0;
    while (true){
        NsApplicationRecord rec; s32 outc = 0;
        if (R_FAILED(nsListApplicationRecord(&rec, 1, offset, &outc)) || outc < 1) break;
        for (int i = 0; i < GAME_COUNT; i++)
            if (rec.application_id == GAMES[i].tid) g_installed[i] = true;
        offset += outc;
    }
    nsExit();
}

static bool readWhole(const char* path, u8** buf, long* len);
static char* g_speciesLines[2048]; static int g_speciesCount = 0;
static char* g_moveLines[1024]; static int g_moveCount = 0;

static void loadAssets(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/species.txt",&raw,&len)) return;
    char* p=(char*)raw;
    long w=0;
    g_speciesLines[0]=p; g_speciesCount=1;
    for(long i=0;i<len;i++){
        char c=p[i];
        if(c=='\r') continue;
        if(c=='\n'){ p[w++]=0; if(g_speciesCount<2048) g_speciesLines[g_speciesCount++]=p+w; }
        else p[w++]=c;
    }
    p[w]=0;
}

static void loadMoves(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/moves.txt",&raw,&len)) return;
    char* p=(char*)raw;
    long w=0;
    g_moveLines[0]=p; g_moveCount=1;
    for(long i=0;i<len;i++){
        char c=p[i];
        if(c=='\r') continue;
        if(c=='\n'){ p[w++]=0; if(g_moveCount<1024) g_moveLines[g_moveCount++]=p+w; }
        else p[w++]=c;
    }
    p[w]=0;
}

static const char* moveName(u16 m){
    if(m>0 && m<g_moveCount && g_moveLines[m][0]) return g_moveLines[m];
    return nullptr;
}

static u8 g_stats[2048][6]; static int g_statsCount = 0;
static void loadStats(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/stats.txt",&raw,&len)) return;
    char* p=(char*)raw; long i=0;
    while(i<len && g_statsCount<2048){
        int v[6] = {0}; int c=0; int acc=0; bool hav=false;
        while(i<len){
            char ch=p[i];
            if(ch>='0'&&ch<='9'){ acc=acc*10+(ch-'0'); hav=true; i++; }
            else if(ch==' '||ch=='\t'){ if(hav){ if(c<6) v[c++]=acc; acc=0; hav=false; } i++; }
            else if(ch=='\n'||ch=='\r'){ if(hav){ if(c<6) v[c++]=acc; acc=0; hav=false; } i++; break; }
            else { i++; }
        }
        if(hav && c<6) v[c++]=acc;
        while(i<len && (p[i]=='\r'||p[i]=='\n')) i++;
        if(c==6) for(int k=0;k<6;k++) g_stats[g_statsCount][k]=(u8)v[k];
        g_statsCount++;
    }
    free(raw);
}
static const u8* baseStats(u16 s){
    if(s>0 && s<g_statsCount) return g_stats[s];
    return nullptr;
}

static u16 g_abilities[2048][3]; static int g_abilitiesCount = 0;
static void loadAbilities(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/abilities.txt",&raw,&len)) return;
    char* p=(char*)raw; long i=0;
    while(i<len && g_abilitiesCount<2048){
        int v[3] = {0}; int c=0; int acc=0; bool hav=false;
        while(i<len){
            char ch=p[i];
            if(ch>='0'&&ch<='9'){ acc=acc*10+(ch-'0'); hav=true; i++; }
            else if(ch==' '||ch=='\t'){ if(hav){ if(c<3) v[c++]=acc; acc=0; hav=false; } i++; }
            else if(ch=='\n'||ch=='\r'){ if(hav){ if(c<3) v[c++]=acc; acc=0; hav=false; } i++; break; }
            else { i++; }
        }
        if(hav && c<3) v[c++]=acc;
        while(i<len && (p[i]=='\r'||p[i]=='\n')) i++;
        if(c==3) for(int k=0;k<3;k++) g_abilities[g_abilitiesCount][k]=(u16)v[k];
        g_abilitiesCount++;
    }
    free(raw);
}
static const u16* baseAbilities(u16 s){
    if(s>0 && s<g_abilitiesCount) return g_abilities[s];
    return nullptr;
}

static u8 g_pp[1024]; static int g_ppCount = 0;
static void loadPP(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/pp.txt",&raw,&len)) return;
    char* p=(char*)raw; long i=0;
    while(i<len && g_ppCount<1024){
        int acc=0; bool hav=false;
        while(i<len){
            char ch=p[i];
            if(ch>='0'&&ch<='9'){ acc=acc*10+(ch-'0'); hav=true; i++; }
            else { if(hav){ g_pp[g_ppCount++] = (u8)acc; } break; }
        }
        while(i<len && (p[i]==' '||p[i]=='\t'||p[i]=='\r'||p[i]=='\n')) i++;
    }
    free(raw);
}

static u8* g_learnsets = nullptr;
static long g_learnsetsLen = 0;
static void loadLearnsets(){
    readWhole("sdmc:/pkhex-nx-assets/learnsets.bin", &g_learnsets, &g_learnsetsLen);
}

static u8 g_growth[2048]; static int g_growthCount = 0;
static void loadGrowth(){
    u8* raw=nullptr; long len=0;
    if(!readWhole("sdmc:/pkhex-nx-assets/growth.txt",&raw,&len)) return;
    char* p=(char*)raw; long i=0;
    while(i<len && g_growthCount<2048){
        int acc=0; bool hav=false;
        while(i<len){
            char ch=p[i];
            if(ch>='0'&&ch<='9'){ acc=acc*10+(ch-'0'); hav=true; i++; }
            else { if(hav){ g_growth[g_growthCount++]=(u8)acc; } break; }
        }
        while(i<len && (p[i]==' '||p[i]=='\t'||p[i]=='\r'||p[i]=='\n')) i++;
    }
    free(raw);
}
static u8 growthOf(u16 s){
    if(s>0 && s<g_growthCount) return g_growth[s];
    return 0;
}
static u32 expForLevel(u32 n, u8 g){
    u32 v;
    switch(g){
        case 1: // Erratic
            if (n<=50) v = n*n*n*(100-n)/50;
            else if (n<=68) v = n*n*n*(150-n)/100;
            else if (n<=98) v = n*n*n*((1911-10*n)/3)/500;
            else v = n*n*n*(160-n)/100;
            break;
        case 2: // Fluctuating
            if (n<=15) v = n*n*n*((n+1)/3+24)/50;
            else if (n<=36) v = n*n*n*(n+14)/50;
            else v = n*n*n*(n/2+32)/50;
            break;
        case 3: { // Medium Slow
            s32 sv = (s32)((6*n*n*n)/5) - (s32)(15*n*n) + (s32)(100*n) - 140;
            v = (sv < 0) ? 0 : (u32)sv;
            break;
        }
        case 4: v = 4*n*n*n/5; break;   // Fast
        case 5: v = 5*n*n*n/4; break;   // Slow
        default: v = n*n*n; break;      // Medium Fast
    }
    return v;
}

// Helper to get ALL legal moves for a species at or below a level
static int getAllLegalMoves(u16 species, u16 level, u16* out_moves, int max_out) {
    int count = 0;
    if (!g_learnsets || g_learnsetsLen < 4 + (species+1)*2) return 0;
    u16 total_species = g_learnsets[2] | (g_learnsets[3] << 8);
    if (species >= total_species) return 0;
    
    u32 off = g_learnsets[4 + species*2] | (g_learnsets[5 + species*2] << 8);
    u32 next_off = (species + 1 < total_species) ? (g_learnsets[4 + (species+1)*2] | (g_learnsets[5 + (species+1)*2] << 8)) : (u32)g_learnsetsLen;
    if (next_off <= off) return 0;
    
    u32 num_moves = (next_off - off) / 3;
    for (u32 i = 0; i < num_moves && count < max_out; i++) {
        u16 mv = g_learnsets[off + i*2] | (g_learnsets[off + i*2 + 1] << 8);
        u8 lv = g_learnsets[off + num_moves*2 + i];
        if (lv <= level && mv != 0) {
            out_moves[count++] = mv;
        }
    }
    return count;
}

static void getLegalMoves(u16 species, u16 level, u16* moves_out){
    moves_out[0] = moves_out[1] = moves_out[2] = moves_out[3] = 0;
    if (!g_learnsets || g_learnsetsLen < 4 + (species+1)*2) {
        moves_out[0] = 33; return;
    }
    u16 count = g_learnsets[2] | (g_learnsets[3] << 8);
    if (species >= count) { moves_out[0] = 33; return; }
    u32 off = g_learnsets[4 + species*2] | (g_learnsets[5 + species*2] << 8);
    u32 next_off = (species + 1 < count) ? (g_learnsets[4 + (species+1)*2] | (g_learnsets[5 + (species+1)*2] << 8)) : (u32)g_learnsetsLen;
    if (next_off <= off) { moves_out[0] = 33; return; }
    u32 num_moves = (next_off - off) / 3;
    if (num_moves == 0) { moves_out[0] = 33; return; }
    static u16 legal_moves[64];
    int n_legal = 0;
    for (u32 i = 0; i < num_moves; i++){
        u16 mv = g_learnsets[off + i*2] | (g_learnsets[off + i*2 + 1] << 8);
        u8 lv = g_learnsets[off + num_moves*2 + i];
        if (lv <= level && mv != 0){
            if (n_legal < 64) legal_moves[n_legal++] = mv;
        }
    }
    if (n_legal == 0) { moves_out[0] = 33; return; }
    int start = (n_legal > 4) ? n_legal - 4 : 0;
    int count_out = 0;
    for (int i = start; i < n_legal && count_out < 4; i++){
        moves_out[count_out++] = legal_moves[i];
    }
}

static bool containsCI(const char* hay, const char* needle){
    if(!needle[0]) return true;
    for(const char* h=hay; *h; h++){
        const char *hh=h, *nn=needle;
        while(*hh && *nn && tolower((unsigned char)*hh)==tolower((unsigned char)*nn)){ hh++; nn++; }
        if(!*nn) return true;
    }
    return false;
}
static const char* speciesName(u16 s){
    if(!g_speciesCount) return nullptr;
    bool zeroBased = containsCI(g_speciesLines[0], "Bulbasaur");
    int idx = zeroBased ? (int)s-1 : (int)s;
    if(idx>=0 && idx<g_speciesCount && g_speciesLines[idx][0]) return g_speciesLines[idx];
    return nullptr;
}
static bool promptText(const char* title, char* out, size_t cap){
    SwkbdConfig k;
    if (R_FAILED(swkbdCreate(&k, 0))) return false;
    swkbdConfigMakePresetDefault(&k);
    swkbdConfigSetHeaderText(&k, title);
    swkbdConfigSetStringLenMax(&k, 32);
    out[0]=0;
    Result rc = swkbdShow(&k, out, (int)cap);
    swkbdClose(&k);
    return R_SUCCEEDED(rc) && out[0];
}

static inline void WLE32(u8* p, u32 v);
static inline void WLE16(u8* p, u16 v);

static u64 waitBtn();
static void pauseA();

// M11 Step 3: The Move Picker UI
static bool movePicker(u16 species, u16 level, u16* out_moves) {
    static u16 available[128];
    int num_avail = getAllLegalMoves(species, level, available, 128);
    
    if (num_avail == 0) {
        printf("\x1b[2J\x1b[0;0HNo legal moves found for this species/level.\n");
        pauseA();
        return false;
    }

    // Initialize with the last 4 legal moves (default behavior)
    for (int i = 0; i < 4; i++) out_moves[i] = 0;
    int start_idx = (num_avail > 4) ? num_avail - 4 : 0;
    for (int i = 0; i < 4 && (start_idx + i) < num_avail; i++) {
        out_moves[i] = available[start_idx + i];
    }

    int slot = 0; // Which of the 4 slots we are editing (0-3)
    
    while (true) {
        printf("\x1b[2J\x1b[0;0H");
        printf("== PICK MOVES (%s L%u) ==\n\n", speciesName(species), level);
        
        for (int i = 0; i < 4; i++) {
            const char* mn = moveName(out_moves[i]);
            char buf[32];
            if (out_moves[i] == 0) snprintf(buf, sizeof(buf), "(empty)");
            else if (!mn) snprintf(buf, sizeof(buf), "Move #%u", out_moves[i]);
            else snprintf(buf, sizeof(buf), "%s", mn);
            
            printf("%s Slot %d: %s\n", (i == slot) ? ">" : " ", i + 1, buf);
        }
        
        printf("\nControls:\n");
        printf("^ v : Select Slot\n");
        printf("< > : Change Move in Slot\n");
        printf("[A] Confirm   [B] Cancel (use defaults)\n");
        consoleUpdate(NULL);
        
        u64 k = waitBtn();
        if (k & HidNpadButton_B) return false; // Cancelled, generatePK9 will use defaults
        if (k & HidNpadButton_A) return true;  // Confirmed
        
        if (k & HidNpadButton_Up && slot > 0) slot--;
        if (k & HidNpadButton_Down && slot < 3) slot++;
        
        if (k & HidNpadButton_Left || k & HidNpadButton_Right) {
            // Find current index in available list
            int cur_idx = -1;
            for (int i = 0; i < num_avail; i++) {
                if (available[i] == out_moves[slot]) { cur_idx = i; break; }
            }
            
            if (k & HidNpadButton_Left) {
                cur_idx = (cur_idx <= 0) ? num_avail - 1 : cur_idx - 1;
            } else {
                cur_idx = (cur_idx >= num_avail - 1) ? 0 : cur_idx + 1;
            }
            out_moves[slot] = available[cur_idx];
        }
    }
}

static void generatePK9(u8* out344, u16 species, u16 level, u8 nature, u8 ball, u32 id32, const char* otName, bool egg, u16* forcedMoves = nullptr){
    memset(out344, 0, 344);
    if (egg) level = 1;   // eggs are always level 1

    u32 ec = (u32)rand();
    WLE32(out344 + 0, ec);
    WLE16(out344 + 8, species);
    WLE16(out344 + 12, (u16)(id32 & 0xFFFF));
    WLE16(out344 + 14, (u16)(id32 >> 16));
    u32 exp = expForLevel(level, growthOf(species));
    WLE32(out344 + 16, exp);

    const u16* abs = baseAbilities(species);
    if (abs){
        int pick = rand() % 3;
        u16 abil_id = abs[0];
        u8 abil_num = 1;
        if (abs[pick] != 0) {
            abil_id = abs[pick];
            abil_num = (u8)(1 << pick);
        } else if (abs[1] != 0) {
            abil_id = abs[1];
            abil_num = 2;
        }
        WLE16(out344 + 20, abil_id);
        out344[22] = abil_num;
    }

    u32 pid = (u32)rand();
    WLE32(out344 + 28, pid);
    out344[0x20] = nature;
    out344[0x21] = nature;
    out344[0x22] = (u8)((rand() % 2) << 1);

    const char* sn = speciesName(species);
    if (sn) for (int i = 0; i < 12 && sn[i]; i++)
        WLE16(out344 + 0x58 + i * 2, (u8)sn[i]);

    u16 moves[4] = {0};
    if (forcedMoves) {
        for (int i = 0; i < 4; i++) moves[i] = forcedMoves[i];
    } else {
        getLegalMoves(species, level, moves);
    }
    for (int i = 0; i < 4; i++){
        WLE16(out344 + 0x72 + i*2, moves[i]);
        u8 pp = (moves[i] > 0 && moves[i] < g_ppCount) ? g_pp[moves[i]] : 0;
        out344[0x7A + i] = pp;
    }

    // IVs all 31; bit30 = egg flag
    WLE32(out344 + 0x8C, 0x3FFFFFFF | (egg ? (1u<<30) : 0));

    out344[0x94] = 0;
    out344[0x95] = 19;

    for (int i = 0; i < 12 && otName[i]; i++)
        WLE16(out344 + 0xF8 + i * 2, (u8)otName[i]);

    out344[0x10C] = egg ? 10 : 70;   // egg cycles (egg) / friendship (normal)

    out344[0x11C] = 23;
    out344[0x11D] = 8;
    out344[0x11E] = 0x0B;
    out344[0x11F] = 0x4C;

    out344[0x124] = ball;
    out344[0x125] = (u8)level;

    out344[0x148] = (u8)level;
    const u8* bs = baseStats(species);
    if (bs){
        u16 maxhp = (u16)(((2*bs[0] + 31) * level) / 100) + level + 10;
        WLE16(out344 + 0x8A, maxhp);
        WLE16(out344 + 0x14A, maxhp);
        WLE16(out344 + 0x14C, (u16)(((2*bs[1] + 31) * level) / 100) + 5);
        WLE16(out344 + 0x14E, (u16)(((2*bs[2] + 31) * level) / 100) + 5);
        WLE16(out344 + 0x150, (u16)(((2*bs[3] + 31) * level) / 100) + 5);
        WLE16(out344 + 0x152, (u16)(((2*bs[4] + 31) * level) / 100) + 5);
        WLE16(out344 + 0x154, (u16)(((2*bs[5] + 31) * level) / 100) + 5);
        WLE16(out344 + 0x156, maxhp);
    } else {
        u16 hp = (u16)(level + 60);
        WLE16(out344 + 0x8A, hp);
        WLE16(out344 + 0x14A, hp);
        WLE16(out344 + 0x156, hp);
    }
}

static const u32 SHA_K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };
struct Sha256 {
    u8 buf[64]; u32 datalen; u64 bitlen; u32 h[8];
    void init(){ datalen=0; bitlen=0;
        h[0]=0x6a09e667;h[1]=0xbb67ae85;h[2]=0x3c6ef372;h[3]=0xa54ff53a;
        h[4]=0x510e527f;h[5]=0x9b05688c;h[6]=0x1f83d9ab;h[7]=0x5be0cd19; }
    static u32 rotr(u32 x,u32 n){ return (x>>n)|(x<<(32-n)); }
    void transform(const u8* p){
        u32 w[64];
        for(int i=0;i<16;i++) w[i]=((u32)p[i*4]<<24)|((u32)p[i*4+1]<<16)|((u32)p[i*4+2]<<8)|(u32)p[i*4+3];
        for(int i=16;i<64;i++){ u32 s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3);
            u32 s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
        u32 a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],x=h[7];
        for(int i=0;i<64;i++){ u32 S1=rotr(e,6)^rotr(e,11)^rotr(e,25); u32 ch=(e&f)^(~e&g);
            u32 t1=x+S1+ch+SHA_K[i]+w[i]; u32 S0=rotr(a,2)^rotr(a,13)^rotr(a,22);
            u32 maj=(a&b)^(a&c)^(b&c); u32 t2=S0+maj; x=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2; }
        h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=x; }
    void update(const u8* p,size_t len){ for(size_t i=0;i<len;i++){ buf[datalen++]=p[i];
        if(datalen==64){ transform(buf); bitlen+=512; datalen=0; } } }
    void final(u8 out[32]){ u32 i=datalen;
        if(datalen<56){ buf[i++]=0x80; while(i<56) buf[i++]=0; }
        else { buf[i++]=0x80; while(i<64) buf[i++]=0; transform(buf); memset(buf,0,56); }
        bitlen += (u64)datalen*8;
        buf[63]=(u8)bitlen; buf[62]=(u8)(bitlen>>8); buf[61]=(u8)(bitlen>>16); buf[60]=(u8)(bitlen>>24);
        buf[59]=(u8)bitlen>>32; buf[58]=(u8)(bitlen>>40); buf[57]=(u8)(bitlen>>48); buf[56]=(u8)(bitlen>>56);
        transform(buf);
        for(int j=0;j<8;j++){ out[j*4]=(u8)(h[j]>>24); out[j*4+1]=(u8)(h[j]>>16); out[j*4+2]=(u8)(h[j]>>8); out[j*4+3]=(u8)h[j]; } }
};

static const u8 INTRO[64] = {
    0x9E,0xC9,0x9C,0xD7,0x0E,0xD3,0x3C,0x44,0xFB,0x93,0x03,0xDC,0xEB,0x39,0xB4,0x2A,
    0x19,0x47,0xE9,0x63,0x4B,0xA2,0x33,0x44,0x16,0xBF,0x82,0xA2,0xBA,0x63,0x55,0xB6,
    0x3D,0x9D,0xF2,0x4B,0x5F,0x7B,0x6A,0xB2,0x62,0x1D,0xC2,0x1B,0x68,0xE5,0xC8,0xB5,
    0x3A,0x05,0x90,0x00,0xE8,0xA8,0x10,0x3D,0xE2,0xEC,0xF0,0x0C,0xB2,0xED,0x4F,0x6D };
static const u8 OUTRO[64] = {
    0xD6,0xC0,0x1C,0x59,0x8B,0xC8,0xB8,0xCB,0x46,0xE1,0x53,0xFC,0x82,0x8C,0x75,0x75,
    0x13,0xE0,0x45,0xDF,0x32,0x69,0x3C,0x75,0xF0,0x59,0xF8,0xD9,0xA2,0x5F,0xB2,0x17,
    0xE0,0x80,0x52,0xDB,0xEA,0x89,0x73,0x99,0x75,0x79,0xAF,0xCB,0x2E,0x80,0x07,0xE6,
    0xF1,0x26,0xE0,0x03,0x0A,0xE6,0x6F,0xF6,0x41,0xBF,0x7E,0x59,0xC2,0xAE,0x55,0xFD };
static const u8 XORPAD[127] = {
    0xA0,0x92,0xD1,0x06,0x07,0xDB,0x32,0xA1,0xAE,0x01,0xF5,0xC5,0x1E,0x84,0x4F,0xE3,
    0x53,0xCA,0x37,0xF4,0xA7,0xB0,0x4D,0xA0,0x18,0xB7,0xC2,0x97,0xDA,0x5F,0x53,0x2B,
    0x75,0xFA,0x48,0x16,0xF8,0xD4,0x8A,0x6F,0x61,0x05,0xF4,0xE2,0xFD,0x04,0xB5,0xA3,
    0x0F,0xFC,0x44,0x92,0xCB,0x32,0xE6,0x1B,0xB9,0xB1,0x2E,0x01,0xB0,0x56,0x53,0x36,
    0xD2,0xD1,0x50,0x3D,0xDE,0x5B,0x2E,0x0E,0x52,0xFD,0xDF,0x2F,0x7B,0xCA,0x63,0x50,
    0xA4,0x67,0x5D,0x23,0x17,0xC0,0x52,0xE1,0xA6,0x30,0x7C,0x2B,0xB6,0x70,0x36,0x5B,
    0x2A,0x27,0x69,0x33,0xF5,0x63,0x7B,0x36,0x3F,0x26,0x9B,0xA3,0xED,0x7A,0x53,0x00,
    0xA4,0x48,0xB3,0x50,0x9E,0x14,0xA0,0x52,0xDE,0x7E,0x10,0x2B,0x1B,0x77,0x6E };

struct SCXorShift32 {
    u32 state; int counter;
    static u32 advance(u32 s){ s^=s<<2; s^=s>>15; s^=s<<13; return s; }
    explicit SCXorShift32(u32 seed):counter(0){ u32 s=seed; int pop=__builtin_popcount(s);
        for(int i=0;i<pop;i++) s=advance(s); state=s; }
    u8 next(){ int c=counter; u8 r=(u8)(state>>(c<<3));
        if(c==3){ state=advance(state); counter=0; } else counter++; return r; }
    u32 next32(){ return (u32)next()|((u32)next()<<8)|((u32)next()<<16)|((u32)next()<<24); }
};
static int scTypeSize(u8 t){ switch(t){ case 3: case 8: case 12: return 1;
    case 9: case 13: return 2; case 10: case 14: case 16: return 4;
    case 11: case 15: case 17: return 8; default: return -1; } }
static inline u32 LE32(const u8* p){ return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24); }
static inline void WLE32(u8* p,u32 v){ p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24); }
static inline u16 LE16(const u8* p){ return (u16)(p[0]|(p[1]<<8)); }
static inline void WLE16(u8* p,u16 v){ p[0]=(u8)v; p[1]=(u8)(v>>8); }

struct BlockInfo { u32 key; u8 type; u8 subtype; u8* data; u32 len; };
static BlockInfo* g_blocks = nullptr; static int g_blockCount = 0;
static BlockInfo* findBlock(u32 key){ for(int i=0;i<g_blockCount;i++) if(g_blocks[i].key==key) return &g_blocks[i]; return nullptr; }

static bool parseBlocks(u8* payload, size_t len, BlockInfo* tbl, int cap, int* count){
    size_t off = 0; *count = 0;
    while (off < len) {
        if (off + 5 > len) return false;
        u32 key = LE32(payload+off); off += 4;
        SCXorShift32 xk(key);
        u8 type = payload[off++] ^ xk.next();
        u8 subtype = 0; u8* bd = nullptr; u32 bl = 0;
        if (type>=1 && type<=3) { }
        else if (type==4){ if(off+4>len) return false; u32 n=LE32(payload+off)^xk.next32(); off+=4;
            if(off+n>len) return false; bd=payload+off; bl=n; for(u32 i=0;i<n;i++) bd[i]^=xk.next(); off+=n; }
        else if (type==5){ if(off+5>len) return false; u32 cnt=LE32(payload+off)^xk.next32(); off+=4;
            subtype=payload[off++]^xk.next(); int sz=scTypeSize(subtype); if(sz<0) return false;
            u64 n=(u64)cnt*sz; if(off+n>len) return false; bd=payload+off; bl=(u32)n;
            for(u32 i=0;i<bl;i++) bd[i]^=xk.next(); off+=n; }
        else { int sz=scTypeSize(type); if(sz<0||off+(size_t)sz>len) return false;
            bd=payload+off; bl=(u32)sz; for(int i=0;i<sz;i++) bd[i]^=xk.next(); off+=(size_t)sz; }
        if (*count < cap) tbl[(*count)++] = { key, type, subtype, bd, bl };
    }
    return true;
}

static bool serialize(u8* out, size_t cap, size_t* outLen){
    size_t off = 0;
    for (int i=0;i<g_blockCount;i++){
        BlockInfo& b = g_blocks[i];
        size_t need = 5 + b.len + (b.type==4?4:(b.type==5?5:0));
        if (off + need + 32 > cap) return false;
        WLE32(out+off, b.key); off += 4;
        SCXorShift32 xk(b.key);
        out[off++] = b.type ^ xk.next();
        if (b.type==4){ WLE32(out+off, b.len ^ xk.next32()); off+=4; }
        else if (b.type==5){ int sz=scTypeSize(b.subtype); if(sz<=0) return false;
            WLE32(out+off, (b.len/sz) ^ xk.next32()); off+=4; out[off++]=b.subtype ^ xk.next(); }
        for (u32 j=0;j<b.len;j++) out[off++] = b.data[j] ^ xk.next();
    }
    for (size_t i=0;i<off;i++) out[i] ^= XORPAD[i % 127];
    Sha256 sha; sha.init(); sha.update(INTRO,64); sha.update(out,off); sha.update(OUTRO,64);
    sha.final(out+off);
    *outLen = off + 32;
    return true;
}

static bool readWhole(const char* path, u8** buf, long* len){
    FILE* f=fopen(path,"rb"); if(!f) return false;
    fseek(f,0,SEEK_END); *len=ftell(f); fseek(f,0,SEEK_SET);
    *buf=(u8*)malloc(*len);
    if(!*buf || fread(*buf,1,*len,f)!=(size_t)*len){ fclose(f); free(*buf); *buf=nullptr; return false; }
    fclose(f); return true;
}
static bool writeWhole(const char* path, const u8* buf, size_t len){
    FILE* f=fopen(path,"wb"); if(!f) return false;
    bool ok = fwrite(buf,1,len,f)==len; fclose(f); return ok;
}
static u32 nowUnix(){ u64 t=0;
    if(R_SUCCEEDED(timeInitialize())){ timeGetCurrentTime(TimeType_LocalSystemClock,&t); timeExit(); }
    return (u32)t; }
static bool findLatestBackup(char* outPath, size_t cap){
    DIR* d=opendir("sdmc:/pkhex-nx-backups"); if(!d) return false;
    struct dirent* ent; char best[64]={0};
    char pre[16]; snprintf(pre,sizeof(pre),"%s_main_",g_game->tag);
    bool legacy = (g_game->tid == GAMES[0].tid);
    while((ent=readdir(d))){
        if (strstr(ent->d_name,"prerestore")) continue;
        bool hit = strncmp(ent->d_name,pre,strlen(pre))==0;
        if (!hit && legacy && strncmp(ent->d_name,"main_",5)==0) hit = true;
        if (hit && strstr(ent->d_name,".bak") && strcmp(ent->d_name,best)>0)
            strncpy(best,ent->d_name,63);
    }
    closedir(d);
    if(!best[0]) return false;
    snprintf(outPath,cap,"sdmc:/pkhex-nx-backups/%s",best);
    return true;
}

static PadState g_pad;
static u64 waitBtn(){ while(true){ padUpdate(&g_pad); u64 d=padGetButtonsDown(&g_pad);
    if(d) return d; consoleUpdate(NULL); } }
static void pauseA(){ printf("\n[A] back\n"); consoleUpdate(NULL); while(!(waitBtn()&HidNpadButton_A)){} }

static bool editU32(const char* title, u32 start, u32 maxVal, u32* out){
    int nd=0; u32 t=maxVal; while(t){ nd++; t/=10; }
    int d[10]; u32 v=start;
    for(int i=nd-1;i>=0;i--){ d[i]=v%10; v/=10; }
    int sel=0;
    while(true){
        printf("\x1b[2J\x1b[0;0H%s\n\n", title);
        for(int i=0;i<nd;i++) printf(i==sel?"[%d]":" %d ", d[i]);
        printf("\n\n< > digit   ^ v value   [A] ok   [B] cancel");
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_Left  && sel>0)    sel--;
        if (k & HidNpadButton_Right && sel<nd-1) sel++;
        if (k & HidNpadButton_Up)   d[sel]=(d[sel]+1)%10;
        if (k & HidNpadButton_Down) d[sel]=(d[sel]+9)%10;
        if (k & HidNpadButton_B) return false;
        if (k & HidNpadButton_A){ u32 val=0; for(int i=0;i<nd;i++) val=val*10+d[i];
            if(val>maxVal) val=maxVal; *out=val; return true; }
    }
}
static const char* CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -._'";
static bool editName(const char* title, const char* start, char* out){
    char buf[13]; memset(buf,0,sizeof(buf)); strncpy(buf,start,12);
    int pos=0; size_t cslen=strlen(CHARSET);
    while(true){
        printf("\x1b[2J\x1b[0;0H%s\n\n", title);
        for(int i=0;i<12;i++) printf(i==pos?"[%c]":" %c ", buf[i]?buf[i]:'.');
        printf("\n\n< > pos   ^ v char   [A] ok   [B] cancel");
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_Left  && pos>0)  pos--;
        if (k & HidNpadButton_Right && pos<11) pos++;
        if (k & (HidNpadButton_Up|HidNpadButton_Down)){
            const char* p = strchr(CHARSET, buf[pos]);
            size_t idx = p ? (size_t)(p-CHARSET) : 0;
            if (k & HidNpadButton_Up) idx=(idx+1)%cslen; else idx=(idx+cslen-1)%cslen;
            buf[pos]=CHARSET[idx];
        }
        if (k & HidNpadButton_B) return false;
        if (k & HidNpadButton_A){ memcpy(out,buf,13); return true; }
    }
}
static void getName(BlockInfo* ms, char* out){
    int n=0;
    for(int i=0;i<12;i++){ u16 c=(u16)(ms->data[0x10+i*2]|(ms->data[0x10+i*2+1]<<8));
        if(!c) break; out[n++]=(c<0x80)?(char)c:'?'; }
    out[n]=0;
}
static void setName(BlockInfo* ms, const char* s){
    for(int i=0;i<12;i++){ u16 c=(u8)s[i];
        ms->data[0x10+i*2]=(u8)c; ms->data[0x10+i*2+1]=0;
        if(!c) break; }
}

static void commitToNand(u8* out, size_t outLen){
    printf("\x1b[2J\x1b[0;0HCOMMIT TO NAND (%s)\n\nMake sure the game is fully closed (not suspended).\n[A] continue   [B] cancel\n", g_game->name);
    consoleUpdate(NULL);
    if (!(waitBtn() & HidNpadButton_A)) return;

    printf("\nStep 1/5: backing up current save to SD...\n"); consoleUpdate(NULL);
    mkdir("sdmc:/pkhex-nx-backups", 0777);
    char bpath[160]; snprintf(bpath,sizeof(bpath),"sdmc:/pkhex-nx-backups/%s_main_%u.bak", g_game->tag, nowUnix());
    u8* raw=nullptr; long rawLen=0;
    bool ok = readWhole("save:/main",&raw,&rawLen);
    if (ok) ok = writeWhole(bpath, raw, (size_t)rawLen);
    if (ok){ u8* vb=nullptr; long vl=0;
        ok = readWhole(bpath,&vb,&vl) && vl==rawLen && memcmp(vb,raw,vl)==0; free(vb); }
    if (!ok){ printf("\nBACKUP FAILED - NAND UNTOUCHED.\n"); free(raw); pauseA(); return; }
    printf("Backup verified: %s\nStep 2/5: writing main...\n", bpath); consoleUpdate(NULL);

    ok = writeWhole("save:/main", out, outLen);
    if (ok){ printf("Step 3/5: writing backup...\n"); consoleUpdate(NULL);
        ok = writeWhole("save:/backup", out, outLen); }
    if (ok){ printf("Step 4/5: committing filesystem...\n"); consoleUpdate(NULL); fsdevCommitDevice("save"); }
    if (ok){ printf("Step 5/5: verifying NAND readback...\n"); consoleUpdate(NULL);
        u8* rb=nullptr; long rl=0; ok = readWhole("save:/main",&rb,&rl);
        ok = ok && rl==(long)outLen && memcmp(rb,out,rl)==0; free(rb);
        rb=nullptr; rl=0;
        ok = ok && readWhole("save:/backup",&rb,&rl) && rl==(long)outLen && memcmp(rb,out,rl)==0; free(rb); }
    free(raw);
    printf(ok ? "\nNAND WRITE OK - boot the game to see your edits!\n"
              : "\nNAND WRITE FAILED - use [R] to restore!\n");
    pauseA();
}

static void restoreFromBackup(){
    printf("\x1b[2J\x1b[0;0H");
    char bpath[160];
    if (!findLatestBackup(bpath,sizeof(bpath))){ printf("No %s backups found.\n", g_game->tag); pauseA(); return; }
    printf("Restore newest %s backup:\n%s\n\n[A] continue   [B] cancel\n", g_game->name, bpath); consoleUpdate(NULL);
    if (!(waitBtn() & HidNpadButton_A)) return;
    u8* bb=nullptr; long bl=0;
    bool ok = readWhole(bpath,&bb,&bl);
    if (ok){
        char pr[160]; snprintf(pr,sizeof(pr),"sdmc:/pkhex-nx-backups/%s_prerestore_%u.bak", g_game->tag, nowUnix());
        u8* cur=nullptr; long cl=0;
        ok = readWhole("save:/main",&cur,&cl) && writeWhole(pr,cur,(size_t)cl);
        free(cur);
        if (!ok) printf("PRE-RESTORE BACKUP FAILED - NAND UNTOUCHED.\n");
    }
    if (ok){ ok = writeWhole("save:/main", bb, (size_t)bl);
        if (ok) ok = writeWhole("save:/backup", bb, (size_t)bl);
        if (ok) fsdevCommitDevice("save"); }
    free(bb);
    printf(ok ? "\nRESTORE OK - press [+] to reload via game selector.\n" : "\nRESTORE FAILED.\n");
    pauseA();
}

static const u8 BLOCKPOS[32][4] = {
    {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,3,1,2},{0,2,3,1},{0,3,2,1},{1,0,2,3},{1,0,3,2},
    {2,0,1,3},{3,0,1,2},{2,0,3,1},{3,0,2,1},{1,2,0,3},{1,3,0,2},{2,1,0,3},{3,1,0,2},
    {2,3,0,1},{3,2,0,1},{1,2,3,0},{1,3,2,0},{2,1,3,0},{3,1,2,0},{2,3,1,0},{3,2,1,0},
    {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,3,1,2},{0,2,3,1},{0,3,2,1},{1,0,2,3},{1,0,3,2} };
static const u8 BLOCKINV[32] = { 0,1,2,4,3,5,6,7,12,18,13,19,8,10,14,20,16,22,9,11,15,21,17,23,
                                 0,1,2,4,3,5,6,7 };

static void cryptArray(u8* d, size_t len, u32 seed){
    for (size_t i = 0; i + 1 < len; i += 2){
        seed = 0x41C64E6Du * seed + 0x00006073u;
        u16 x = (u16)(seed >> 16);
        u16 v = (u16)(d[i] | (d[i+1] << 8));
        v ^= x;
        d[i] = (u8)v; d[i+1] = (u8)(v >> 8);
    }
}
static void shuffle8(u8* d, u32 sv){
    if (sv == 0) return;
    u8 perm[4] = {0,1,2,3}, slotOf[4] = {0,1,2,3};
    for (int i = 0; i < 3; i++){
        u8 desired = BLOCKPOS[sv][i];
        u8 j = slotOf[desired];
        if (j == i) continue;
        u8* a = d + i*80; u8* b = d + j*80;
        for (int t = 0; t < 80; t++){ u8 tmp=a[t]; a[t]=b[t]; b[t]=tmp; }
        u8 blockAtI = perm[i];
        perm[j] = blockAtI;
        slotOf[blockAtI] = j;
    }
}
static void decrypt8(u8* d, size_t len){
    u32 pv = LE32(d);
    u32 sv = (pv >> 13) & 31;
    cryptArray(d + 8, 0x148 - 8, pv);
    if (len > 0x148) cryptArray(d + 0x148, len - 0x148, pv);
    shuffle8(d + 8, sv);
}
static void encrypt8(u8* d, size_t len){
    u32 pv = LE32(d);
    u32 sv = BLOCKINV[(pv >> 13) & 31];
    shuffle8(d + 8, sv);
    cryptArray(d + 8, 0x148 - 8, pv);
    if (len > 0x148) cryptArray(d + 0x148, len - 0x148, pv);
}
static void healChecksum(u8* d){
    u32 sum = 0;
    for (size_t i = 8; i < 0x148; i += 2) sum += (u16)(d[i] | (d[i+1] << 8));
    WLE16(d + 6, (u16)sum);
}

static const char* NATURES[25] = { "Hardy","Lonely","Brave","Adamant","Naughty","Bold","Docile",
    "Relaxed","Impish","Lax","Timid","Hasty","Serious","Jolly","Naive","Modest","Mild","Quiet",
    "Bashful","Rash","Calm","Gentle","Sassy","Careful","Quirky" };
static const char* TYPES[18] = { "Normal","Fighting","Flying","Poison","Ground","Rock","Bug",
    "Ghost","Steel","Fire","Water","Grass","Electric","Psychic","Ice","Dragon","Dark","Fairy" };
static const char* typeName(u8 t){ if (t < 18) return TYPES[t]; if (t == 18) return "Stellar"; return "??"; }
static const char* STATNAMES[6] = { "HP","ATK","DEF","SPE","SPA","SPD" };
static void readUtf16(const u8* p, int max, char* out){
    int n = 0;
    for (int i = 0; i < max; i++){
        u16 c = (u16)(p[i*2] | (p[i+1] << 8));
        if (!c) break;
        out[n++] = (c < 0x80) ? (char)c : '?';
    }
    out[n] = 0;
}
static void readNameSmart(const u8* p, int max, char* out){
    int n=0; bool bad=false;
    for(int i=0;i<max;i++){
        u16 c=(u16)(p[i*2]|(p[i+2+0]<<8));
        if(!c) break;
        if(c>=0x80){ bad=true; break; }
        out[n++]=(char)c;
    }
    out[n]=0;
    if(!bad && n>0) return;
    n=0;
    for(int i=0;i<max*2;i++){
        u8 c=p[i];
        if(!c) break;
        if(c>=0x20 && c<0x7F) out[n++]=(char)c;
    }
    out[n]=0;
}
static u32 getIV(u8* b, int i){ return (LE32(b+0x8C) >> (i*5)) & 31; }
static void setIV(u8* b, int i, u32 v){
    if (v > 31) v = 31;
    u32 iv = LE32(b+0x8C);
    iv &= ~(0x1Fu << (i*5));
    iv |= v << (i*5);
    WLE32(b+0x8C, iv);
}
static void toggleShiny(u8* b){
    u16 tid = LE16(b+0x0C), sid = LE16(b+0x0E);
    u32 pid = LE32(b+0x1C);
    u16 lo = (u16)(pid & 0xFFFF);
    u16 hi = (u16)(pid >> 16);
    u16 desired = (u16)(tid ^ sid) & 0xFFF0;
    if (((hi ^ lo) & 0xFFF0) == desired) desired ^= 0x10;
    hi = lo ^ desired;
    WLE32(b+0x1C, ((u32)hi << 16) | lo);
}

static void showSlotDetail(u8* slot){
    u8 buf[PK9_PARTY];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, slot, BOX_STRIDE);
    decrypt8(buf, BOX_STRIDE);
    bool dirty = false;
    while (true){
        char nick[13], ot[13];
        readNameSmart(buf + 0x58, 12, nick);
        readNameSmart(buf + 0xF8, 12, ot);
        u16 species = LE16(buf+0x08);
        u32 exp     = LE32(buf + 0x10);
        u32 pid     = LE32(buf + 0x1C);
        u16 tid     = LE16(buf+0x0C), sid = LE16(buf+0x0E);
        u8 nature   = buf[0x20];
        u8 gender   = (buf[0x22] >> 1) & 3;
        u8 teraO = buf[0x94], teraX = buf[0x95];
        u32 psv = ((pid >> 16) ^ (pid & 0xFFFF)) >> 4;
        u32 tsv = (u32)(tid ^ sid) >> 4;
        u32 id32 = (u32)tid | ((u32)sid << 16);
        u32 sum = 0;
        for (size_t i = 8; i < 0x148; i += 2) sum += (u16)(buf[i] | (buf[i+1] << 8));
        u16 chk = LE16(buf+6);
        const char* sn = speciesName(species);
        bool nickFlag = ((LE32(buf+0x8C) >> 31) & 1) != 0;
        const char* displayName = (nickFlag && nick[0]) ? nick : (sn ? sn : nick);

        printf("\x1b[2J\x1b[0;0H");
        printf("== %s (OT: %s) ==%s\n", displayName, ot, dirty ? "  *edited*" : "");
        printf("Species: %s (#%u)   Level: %u\n", sn ? sn : "?", species, buf[0x148]);
        printf("Nature: %s   Gender: %s\n", nature < 25 ? NATURES[nature] : "??",
               gender == 0 ? "M" : gender == 1 ? "F" : "-");
        printf("Tera: %s\n", typeName(teraX != 19 ? teraX : teraO));
        printf("EXP: %u\n", exp);
        printf("IVs  HP/AT/DF/SP/SA/SD: %u/%u/%u/%u/%u/%u\n",
            getIV(buf,0),getIV(buf,1),getIV(buf,2),getIV(buf,3),getIV(buf,4),getIV(buf,5));
        printf("EVs  HP/AT/DF/SP/SA/SD: %u/%u/%u/%u/%u/%u\n",
            buf[0x26],buf[0x27],buf[0x28],buf[0x29],buf[0x2A],buf[0x2B]);
        printf("Shiny: %s   TID %06u / SID %06u\n", psv == tsv ? "YES!" : "no", id32 % 1000000, id32 / 1000000);
        printf("Egg: %s\n", ((LE32(buf+0x8C) >> 30) & 1) ? "YES" : "no");
        printf("Checksum: %s%s\n", (u16)sum == chk ? "OK" : "BAD", dirty ? " (heals on B)" : "");
        printf("\n[X] IVs  [Y] EVs  [A] Nature\n[L] Tera  [R] Shiny  [B] save&back\n");
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_B) break;
        if (k & HidNpadButton_X){
            for (int i = 0; i < 6; i++){
                u32 v; char t[32]; snprintf(t,sizeof(t),"IV %s (0-31)", STATNAMES[i]);
                if (editU32(t, getIV(buf,i), 31, &v)){ setIV(buf,i,v); dirty = true; }
            }
        }
        if (k & HidNpadButton_Y){
            for (int i = 0; i < 6; i++){
                u32 v; char t[32]; snprintf(t,sizeof(t),"EV %s (0-252)", STATNAMES[i]);
                if (editU32(t, buf[0x26+i], 252, &v)){ buf[0x26+i] = (u8)v; dirty = true; }
            }
        }
        if (k & HidNpadButton_A){
            u32 v;
            if (editU32("NATURE (0-24, see list)", nature, 24, &v)){ buf[0x20] = (u8)v; dirty = true; }
        }
        if (k & HidNpadButton_L){
            u32 v;
            if (editU32("TERA OVERRIDE (0-18, 19=none)", teraX, 19, &v)){ buf[0x95] = (u8)v; dirty = true; }
        }
        if (k & HidNpadButton_R){ toggleShiny(buf); dirty = true; }
    }
    if (dirty){
        healChecksum(buf);
        encrypt8(buf, BOX_STRIDE);
        memcpy(slot, buf, BOX_STRIDE);
        printf("\x1b[2J\x1b[0;0HSlot updated in memory (checksum healed, re-encrypted).\nUse [L] on the main menu to commit to NAND.\n");
        pauseA();
    }
}

static bool isEncrypted8(const u8* d){ return LE16(d+0x70)!=0 || LE16(d+0x110)!=0; }
static bool endsWithPk9(const char* s){
    size_t l=strlen(s);
    return l>=4 && s[l-4]=='.' && (s[l-3]=='p'||s[l-3]=='P') && (s[l-2]=='k'||s[l-2]=='K') && s[l-1]=='9';
}
static bool loadPk9File(const char* path, u8* out344){
    u8* buf=nullptr; long len=0;
    if(!readWhole(path,&buf,&len)) return false;
    bool ok=false;
    if(len==PK9_PARTY || len==BOX_STRIDE){
        memset(out344,0,PK9_PARTY);
        memcpy(out344,buf,(size_t)len);
        if(isEncrypted8(out344)) decrypt8(out344,(size_t)len);
        u32 sum=0; for(size_t i=8;i<0x148;i+=2) sum+=LE16(out344+i);
        if((u16)sum==LE16(out344+6)){ healChecksum(out344); encrypt8(out344, BOX_STRIDE); ok=true; }
    }
    free(buf);
    return ok;
}
static bool pickPk9(char* outPath, size_t cap){
    DIR* d=opendir("sdmc:/pkhex-nx-inject");
    if(!d){ printf("\x1b[2J\x1b[0;0HNo sdmc:/pkhex-nx-inject folder.\nCreate it on the SD card and copy .pk9 files in.\n"); pauseA(); return false; }
    char names[12][64]; int n=0;
    struct dirent* ent;
    while((ent=readdir(d)) && n<12){
        if(endsWithPk9(ent->d_name)){ strncpy(names[n],ent->d_name,63); names[n][63]=0; n++; }
    }
    closedir(d);
    if(n==0){ printf("\x1b[2J\x1b[0;0HNo .pk9 files in sdmc:/pkhex-nx-inject\n"); pauseA(); return false; }
    int sel=0;
    while(true){
        printf("\x1b[2J\x1b[0;0HInject which file?\n\n");
        for(int i=0;i<n;i++) printf(i==sel?"> %s\n":"  %s\n", names[i]);
        printf("\n^ v select   [A] choose   [B] cancel\n");
        consoleUpdate(NULL);
        u64 k=waitBtn();
        if(k&HidNpadButton_B) return false;
        if(k&HidNpadButton_Up && sel>0) sel--;
        if(k&HidNpadButton_Down && sel<n-1) sel++;
        if(k&HidNpadButton_A){ snprintf(outPath,cap,"sdmc:/pkhex-nx-inject/%s",names[sel]); return true; }
    }
}

static const u8 BALL_IDS[26] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};
static const char* BALL_NAMES[26] = {"Master","Ultra","Great","Poke","Safari","Net",
    "Dive","Nest","Repeat","Timer","Luxury","Premier","Dusk","Heal","Quick","Cherish",
    "Fast","Level","Lure","Heavy","Love","Friend","Moon","Sport","Dream","Beast"};
static u8 ballPicker(){
    int sel = 0;
    while (true){
        printf("\x1b[2J\x1b[0;0HChoose ball:\n\n");
        for (int i = 0; i < 13; i++){
            printf("%s %-9s Ball", i==sel ? ">" : " ", BALL_NAMES[i]);
            printf("   %s %s Ball\n", (i+13)==sel ? ">" : " ", BALL_NAMES[i+13]);
        }
        printf("\n^ v select   [A] pick   [B] cancel\n");
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_B) return 0;
        if (k & HidNpadButton_Up && sel > 0) sel--;
        if (k & HidNpadButton_Down && sel < 25) sel++;
        if (k & HidNpadButton_A) return BALL_IDS[sel];
    }
}

static void boxViewer(){
    BlockInfo* box = findBlock(KBox);
    if (!box){ printf("No box block!\n"); pauseA(); return; }
    const int SLOTS = 30;
    int boxes = (int)(box->len / (SLOTS * BOX_STRIDE));
    if (boxes > 32) boxes = 32;
    if (boxes < 1){ printf("Box block too small!\n"); pauseA(); return; }
    int curBox = 0, sel = 0, cloneSrc = -1;
    while (true){
        u32 off = (u32)((curBox*SLOTS + sel) * BOX_STRIDE);
        u8* cur = box->data + off;
        bool occ = (off + 4 <= box->len) && LE32(cur) != 0;
        printf("\x1b[2J\x1b[0;0H");
        if (cloneSrc >= 0) printf("CLONE MODE: pick target  [A] paste  [B] cancel\n");
        printf("%s  Box %2d/%d  D-pad:move L/R:box A:open B:back\n", g_game->tag, curBox+1, boxes);
        printf("X:clone Y:delete ZL:inject ZR:export +:search\n\n");
        for (int row = 0; row < 5; row++){
            printf("  ");
            for (int col = 0; col < 6; col++){
                int s = row*6+col;
                u32 o = (u32)((curBox*SLOTS + s) * BOX_STRIDE);
                bool oc = (o + 4 <= box->len) && LE32(box->data + o) != 0;
                printf(s==sel ? (oc?"[PK]":"[..]") : (oc?" PK ":" .. "));
            }
            printf("\n");
        }
        printf("\nSlot %2d: %s  EC=%08X\n", sel, occ ? "occupied" : "(empty)", LE32(cur));
        printf("head: %02X %02X %02X %02X %02X %02X %02X %02X\n",
            cur[0],cur[1],cur[2],cur[3],cur[4],cur[5],cur[6],cur[7]);
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_B){ if (cloneSrc >= 0){ cloneSrc = -1; continue; } return; }
        if (k & HidNpadButton_Left  && sel%6>0) sel--;
        if (k & HidNpadButton_Right && sel%6<5) sel++;
        if (k & HidNpadButton_Up    && sel>=6)  sel-=6;
        if (k & HidNpadButton_Down  && sel<24)  sel+=6;
        if (k & HidNpadButton_L && curBox>0) curBox--;
        if (k & HidNpadButton_R && curBox<boxes-1) curBox++;
        if (k & HidNpadButton_Plus){
            char q[40]={0};
            if (promptText("Search species name", q, sizeof(q))){
                static int hits[960]; int nh=0;
                for (int b=0;b<boxes && nh<960;b++) for (int s2=0;s2<SLOTS && nh<960;s2++){
                    u32 o=(u32)((b*SLOTS+s2)*BOX_STRIDE);
                    if (o+4>box->len || LE32(box->data+o)==0) continue;
                    u8 tmp[PK9_PARTY]; memset(tmp,0,sizeof(tmp)); memcpy(tmp,box->data+o,BOX_STRIDE);
                    decrypt8(tmp,BOX_STRIDE);
                    u16 sp=LE16(tmp+8);
                    const char* nm=speciesName(sp);
                    char fb[16];
                    if(!nm){ snprintf(fb,sizeof(fb),"#%u",sp); nm=fb; }
                    if(containsCI(nm,q)) hits[nh++]=b*SLOTS+s2;
                }
                int sel2=0;
                while(true){
                    printf("\x1b[2J\x1b[0;0HSearch \"%s\": %d found\n\n", q, nh);
                    if(nh==0) printf("(no matches)\n");
                    int page=sel2/10*10;
                    for(int i=0;i<10 && page+i<nh;i++){
                        int g=hits[page+i];
                        u32 o=(u32)(g*BOX_STRIDE);
                        u8 tmp[PK9_PARTY]; memset(tmp,0,sizeof(tmp)); memcpy(tmp,box->data+o,BOX_STRIDE);
                        decrypt8(tmp,BOX_STRIDE);
                        const char* nm=speciesName(LE16(tmp+8));
                        printf("%s Box %2d Slot %2d  %s\n", (page+i)==sel2?">":" ", g/SLOTS+1, g%SLOTS, nm?nm:"?");
                    }
                    printf("\n^ v select   [A] jump   [B] back\n");
                    consoleUpdate(NULL);
                    u64 k2=waitBtn();
                    if(k2&HidNpadButton_B) break;
                    if(k2&HidNpadButton_Up&&sel2>0)sel2--;
                    if(k2&HidNpadButton_Down&&sel2<nh-1)sel2++;
                    if(k2&HidNpadButton_A&&nh){ curBox=hits[sel2]/SLOTS; sel=hits[sel2]%SLOTS; break; }
                }
            }
        }
        if (k & HidNpadButton_A){
            if (cloneSrc >= 0){ memcpy(cur, box->data + cloneSrc, BOX_STRIDE); cloneSrc = -1;
                printf("\x1b[2J\x1b[0;0HCloned. Commit with [L] on the main menu.\n"); pauseA(); continue; }
            if (occ) showSlotDetail(cur);
        }
        if ((k & HidNpadButton_X) && occ) cloneSrc = (int)off;
        if (k & HidNpadButton_Y){
            printf("\x1b[2J\x1b[0;0H%s\n[A] yes   [B] no\n",
                   occ ? "DELETE this Pokemon?" : "RESET this slot to game-empty?");
            consoleUpdate(NULL);
            if (waitBtn() & HidNpadButton_A){
                u8* ref = nullptr;
                const int cands[] = {95, 94, 93, 92, 91};
                for (int i = 0; i < 5 && !ref; i++){
                    u8* s = box->data + cands[i]*BOX_STRIDE;
                    if (LE32(s) == 0 && s[10] != 0) ref = s;
                }
                if (ref) memcpy(cur, ref, BOX_STRIDE);
                else { memset(cur, 0, BOX_STRIDE); encrypt8(cur, BOX_STRIDE); healChecksum(cur); }
                printf("\x1b[2J\x1b[0;0HSlot reset to game-empty. Commit with [L].\n"); pauseA();
            }
        }
        if ((k & HidNpadButton_ZR) && occ){
            mkdir("sdmc:/pkhex-nx-inject", 0777);
            char p[160]; snprintf(p,sizeof(p),"sdmc:/pkhex-nx-inject/export_%08X.pk9", LE32(cur));
            u8 tmp[PK9_PARTY]; memset(tmp,0,sizeof(tmp)); memcpy(tmp,cur,BOX_STRIDE);
            bool ok = writeWhole(p, tmp, PK9_PARTY);
            printf("\x1b[2J\x1b[0;0H%s\n%s\n", ok ? "Exported:" : "Export FAILED:", p); pauseA();
        }
        if (k & HidNpadButton_ZL){
            if (occ){
                printf("\x1b[2J\x1b[0;0HOverwrite this slot?\n[A] yes   [B] no\n"); consoleUpdate(NULL);
                if (!(waitBtn() & HidNpadButton_A)) continue;
            }

            printf("\x1b[2J\x1b[0;0HInject Pokemon:\n\n[A] From .pk9 file\n[X] Generate from database\n[B] Cancel\n");
            consoleUpdate(NULL);
            u64 k2 = waitBtn();

            if (k2 & HidNpadButton_X){
                char q[40]={0};
                if (!promptText("Search species", q, sizeof(q))) continue;

                static int matches[2048]; int nm=0;
                for (int i=0; i<g_speciesCount && nm<2048; i++){
                    if (g_speciesLines[i][0] && containsCI(g_speciesLines[i], q))
                        matches[nm++] = i;
                }
                if (nm == 0){
                    printf("\x1b[2J\x1b[0;0HNo species found matching \"%s\".\n", q);
                    pauseA(); continue;
                }

                int sel3=0; u16 pickedSpecies=0;
                while(true){
                    printf("\x1b[2J\x1b[0;0HSelect species (%d matches):\n\n", nm);
                    int page=sel3/10*10;
                    for(int i=0;i<10 && page+i<nm;i++)
                        printf("%s %s\n", (page+i)==sel3?">":" ", g_speciesLines[matches[page+i]]);
                    printf("\n^ v select   [A] pick   [B] cancel\n");
                    consoleUpdate(NULL);
                    u64 k3=waitBtn();
                    if(k3&HidNpadButton_B) break;
                    if(k3&HidNpadButton_Up&&sel3>0)sel3--;
                    if(k3&HidNpadButton_Down&&sel3<nm-1)sel3++;
                    if(k3&HidNpadButton_A){
                        bool zb = containsCI(g_speciesLines[0], "Bulbasaur");
                        pickedSpecies = (u16)(matches[sel3] + (zb ? 1 : 0));
                        break;
                    }
                }
                if (pickedSpecies == 0) continue;

                u8 pickedBall = ballPicker();
                if (pickedBall == 0) continue;

                u32 level = 50; bool egg = false; 
                u16 forcedMoves[4] = {0};
                bool useForcedMoves = false;
                bool go = false;
                
                while(true){
                    printf("\x1b[2J\x1b[0;0HGenerate %s\n\nLevel: %u\nEgg:   %s\nMoves: %s\n\n[A] set level   [X] toggle egg\n[Y] pick moves   [+] generate   [B] cancel\n",
                           speciesName(pickedSpecies), level, egg?"YES":"no", useForcedMoves?"CUSTOM":"AUTO");
                    consoleUpdate(NULL);
                    u64 k4 = waitBtn();
                    if (k4 & HidNpadButton_B) break;
                    if (k4 & HidNpadButton_A){ u32 v; if(editU32("LEVEL (1-100)", level, 100, &v)) level=v; }
                    if (k4 & HidNpadButton_X) egg = !egg;
                    if (k4 & HidNpadButton_Y){ 
                        if (movePicker(pickedSpecies, (u16)level, forcedMoves)) {
                            useForcedMoves = true;
                        }
                    }
                    if (k4 & HidNpadButton_Plus){ go=true; break; }
                }
                if (!go) continue;

                BlockInfo* ms=findBlock(KMyStatus);
                char otName[13]={0};
                u32 id32=0;
                if(ms){ id32=LE32(ms->data); getName(ms,otName); }

                u8 tmp[PK9_PARTY];
                generatePK9(tmp, pickedSpecies, (u16)level, 0, pickedBall, id32, otName, egg, useForcedMoves ? forcedMoves : nullptr);
                healChecksum(tmp);
                encrypt8(tmp, BOX_STRIDE);
                memcpy(cur, tmp, BOX_STRIDE);
                printf("\x1b[2J\x1b[0;0HGenerated %s (L%u%s, ball %u, moves %s).\nCommit with [L].\n",
                       speciesName(pickedSpecies), egg?1:level, egg?", EGG":"", pickedBall, useForcedMoves?"CUSTOM":"AUTO");
                pauseA();
            }
            else if (k2 & HidNpadButton_A){
                char path[160];
                if (pickPk9(path,sizeof(path))){
                    u8 tmp[PK9_PARTY];
                    if (loadPk9File(path,tmp)){ memcpy(cur,tmp,BOX_STRIDE);
                        printf("\x1b[2J\x1b[0;0HInjected. Commit with [L].\n"); pauseA(); }
                    else { printf("\x1b[2J\x1b[0;0HInvalid .pk9 (bad size or checksum).\n"); pauseA(); }
                }
            }
        }
    }
}

static void rawDumpMainBackup(){
    printf("\x1b[2J\x1b[0;0HRAW DUMP (%s main + backup + block list)\n\n", g_game->tag);
    mkdir("sdmc:/pkhex-nx-rawdumps", 0777);
    u32 t = nowUnix();
    char mp[160], bp[160], tp[160];
    snprintf(mp, sizeof(mp), "sdmc:/pkhex-nx-rawdumps/%s_main_%u.bin", g_game->tag, t);
    snprintf(bp, sizeof(bp), "sdmc:/pkhex-nx-rawdumps/%s_backup_%u.bin", g_game->tag, t);
    snprintf(tp, sizeof(tp), "sdmc:/pkhex-nx-rawdumps/%s_blocks_%u.txt", g_game->tag, t);

    u8* md=nullptr; long msz=0;
    bool hm = readWhole("save:/main", &md, &msz);
    bool wm = false;
    if (hm) wm = writeWhole(mp, md, (size_t)msz);
    free(md);

    u8* bd=nullptr; long bsz=0;
    bool hb = readWhole("save:/backup", &bd, &bsz);
    bool wb = false;
    if (hb) wb = writeWhole(bp, bd, (size_t)bsz);
    free(bd);

    FILE* tf = fopen(tp, "w");
    if (tf) {
        fprintf(tf, "Total blocks: %d\n\n", g_blockCount);
        for (int i = 0; i < g_blockCount; i++) {
            fprintf(tf, "Key: 0x%08X  Type: %2d  Sub: %2d  Len: %8u\n",
                    g_blocks[i].key, g_blocks[i].type, g_blocks[i].subtype, g_blocks[i].len);
        }
        fclose(tf);
    }

    printf("main:   %s\n", wm ? mp : "FAILED");
    printf("backup: %s\n", wb ? bp : "FAILED");
    printf("blocks: %s\n", tf ? tp : "FAILED");
    pauseA();
}

static int gameSelector(){
    int sel = 0;
    while (true){
        printf("\x1b[2J\x1b[0;0H");
        printf("PKHeX-NX M11 - SELECT GAME\n\n");
        for (int i = 0; i < GAME_COUNT; i++)
            printf("%s %s %s\n", i==sel ? ">" : " ", GAMES[i].name,
                   g_installed[i] ? "(detected)" : "(not detected)");
        printf("\n^ v select   [A] open save   [B] exit app\n");
        consoleUpdate(NULL);
        u64 k = waitBtn();
        if (k & HidNpadButton_B) return -1;
        if (k & HidNpadButton_Up && sel > 0) sel--;
        if (k & HidNpadButton_Down && sel < GAME_COUNT-1) sel++;
        if (k & HidNpadButton_A) return sel;
    }
}

int main(int argc, char** argv){
    consoleInit(NULL); appletLockExit();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard); // <--- HID input initialized before guard
    // ---- Applet Mode guard: refuse to run without Title Override ----
    if (appletGetAppletType() != AppletType_Application) {
        PadState guard_pad;
        padInitializeDefault(&guard_pad);
        consoleClear();
        printf("\n\n  !! LAUNCH ERROR !!\n\n");
        printf("  PKHeX-NX was launched from the Album (Applet Mode).\n");
        printf("  This mode blocks save-file access.\n\n");
        printf("  HOW TO FIX:\n");
        printf("  1. Return to the Home Menu.\n");
        printf("  2. Highlight Pokemon Scarlet or Violet.\n");
        printf("  3. HOLD [R] while pressing [A] to launch it.\n");
        printf("  4. PKHeX-NX will open with full access.\n\n");
        printf("  Press [A] to exit.\n");
        consoleUpdate(NULL); // Flush initial text to screen
        
        while (appletMainLoop()) {
            padUpdate(&guard_pad);
            if (padGetButtonsDown(&guard_pad) & HidNpadButton_A) break;
            consoleUpdate(NULL); // Flushes text while waiting
            svcSleepThread(16000000); // Don't burn CPU while waiting
        }
        appletUnlockExit();
        consoleExit(NULL);
        return 0;
    }
    // ---- end guard ----
    Result rc = fsInitialize();
    if (R_FAILED(rc)){ printf("FATAL fs 0x%X\n",rc); while(appletMainLoop()) consoleUpdate(NULL); return 1; }
    fsdevMountSdmc();
    loadAssets();
    loadStats();
    loadAbilities();
    loadPP();
    loadLearnsets();
    loadGrowth();
    loadMoves();
    srand((unsigned int)time(NULL));

    AccountUid uid={0};
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Application))){
        accountGetLastOpenedUser(&uid);
        if(!accountUidIsValid(&uid)){ s32 tot=0; accountListAllUsers(&uid,1,&tot); }
        accountExit(); }

    padConfigureInput(1,HidNpadStyleSet_NpadStandard); padInitializeDefault(&g_pad);

    detectGames();

    while (true){
        int gi = gameSelector();
        if (gi < 0) break;
        g_game = &GAMES[gi];
        KMyStatus = g_game->keyMyStatus; KMoney = g_game->keyMoney; KBox = g_game->keyBox;
        BOX_STRIDE = g_game->boxStride;

        rc = fsdevMountSaveData("save", g_game->tid, uid);
        if (R_FAILED(rc)) rc = fsdevMountDeviceSaveData("save", g_game->tid);
        if (R_FAILED(rc)){
            printf("\x1b[2J\x1b[0;0HMount failed 0x%X\nNo save data found for %s.\n(Make sure the game is fully closed.)\n",
                   rc, g_game->name);
            pauseA(); continue; }

        u8* data=nullptr; long size=0;
        bool ok = readWhole("save:/main",&data,&size);
        if (ok && size > 32){
            size_t payloadLen=(size_t)size-32;
            Sha256 sha; sha.init(); sha.update(INTRO,64); sha.update(data,payloadLen); sha.update(OUTRO,64);
            u8 h[32]; sha.final(h);
            if (memcmp(h,data+payloadLen,32)!=0){ printf("HASH MISMATCH\n"); ok=false; }
            else {
                for (size_t i=0;i<payloadLen;i++) data[i]^=XORPAD[i%127];
                g_blocks=(BlockInfo*)malloc(sizeof(BlockInfo)*32768);
                int cnt=0;
                if(!parseBlocks(data,payloadLen,g_blocks,32768,&cnt)){ printf("parse fail\n"); ok=false; free(g_blocks); g_blocks=nullptr; }
                else g_blockCount=cnt;
            }
        } else ok = false;
        if (!ok){
            printf("Could not parse %s save.\n", g_game->name);
            free(data); pauseA();
            if (fsdevGetDeviceFileSystem("save")) fsdevUnmountDevice("save");
            continue; }

        while(true){
            printf("\x1b[2J\x1b[0;0H");
            printf("PKHeX-NX M11 - %s\n\n", g_game->name);
            BlockInfo* ms=findBlock(KMyStatus); BlockInfo* mo=findBlock(KMoney);
            char name[13]={0};
            if(ms){ u32 full=LE32(ms->data); getName(ms,name);
                printf("Name : %s\nTID  : %06u   SID: %06u\n", name, full%1000000, full/1000000); }
            if(mo) printf("Money: %u\n", LE32(mo->data));
            printf("\n[A] Name  [B] TID/SID  [X] Money\n[Y] Round-trip test (SD only)  [ZR] Raw dump\n[L] COMMIT TO NAND (backup first)\n[R] Restore newest backup\n[-] Box viewer\n[+] Change game\n");
            consoleUpdate(NULL);
            u64 k=waitBtn();

            if (k & HidNpadButton_Plus) break;
            if (ms && (k & HidNpadButton_A)){ char nn[13]; if(editName("EDIT NAME",name,nn)) setName(ms,nn); }
            if (ms && (k & HidNpadButton_B)){ u32 full=LE32(ms->data), tid,sid;
                if(editU32("EDIT TID", full%1000000, 999999, &tid) &&
                   editU32("EDIT SID", full/1000000, 999999, &sid))
                    WLE32(ms->data, sid*1000000+tid); }
            if (mo && (k & HidNpadButton_X)){ u32 m; if(editU32("EDIT MONEY", LE32(mo->data), 9999999, &m)) WLE32(mo->data,m); }
            if ((k & HidNpadButton_Y) || (k & HidNpadButton_L)){
                u8* out=(u8*)malloc(size); size_t outLen=0;
                bool sok = out && serialize(out,(size_t)size,&outLen) && outLen==(size_t)size;
                if(!sok){ printf("serialize failed\n"); free(out); pauseA(); continue; }
                if (k & HidNpadButton_Y){
                    char rp[160]; snprintf(rp,sizeof(rp),"sdmc:/pkhex_%s_roundtrip.main", g_game->tag);
                    sok = writeWhole(rp, out, outLen);
                    printf(sok ? "Round-trip file written (NAND untouched).\n" : "SD write failed.\n");
                    free(out); pauseA();
                } else {
                    commitToNand(out, outLen);
                    free(out);
                }
            }
            if (k & HidNpadButton_R) restoreFromBackup();
            if (k & HidNpadButton_Minus) boxViewer();
            if (k & HidNpadButton_ZR) rawDumpMainBackup();
        }

        free(g_blocks); g_blocks=nullptr; g_blockCount=0;
        free(data); data=nullptr;
        if (fsdevGetDeviceFileSystem("save")) fsdevUnmountDevice("save");
    }

    fsdevUnmountAll(); fsExit();
    appletUnlockExit(); consoleExit(NULL);
    return 0;
}