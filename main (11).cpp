#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <cctype>
#include <vector>
using namespace std;

// === RENKLER ===
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// === ELEMENT SİSTEMİ ===
enum Element { ATES, SU, ELEKTRIK, DOGA, YOK };
string elementAdi(Element e) {
    switch(e) { case ATES: return "Ateş"; case SU: return "Su"; case ELEKTRIK: return "Elektrik"; case DOGA: return "Doğa"; default: return "Yok"; }
}
double elementCarpani(Element a, Element b) {
    if (a == YOK || b == YOK) return 1.0;
    if (a == ATES && b == DOGA) return 1.5;
    if (a == DOGA && b == ELEKTRIK) return 1.5;
    if (a == ELEKTRIK && b == SU) return 1.5;
    if (a == SU && b == ATES) return 1.5;
    if (a == DOGA && b == ATES) return 0.5;
    if (a == ELEKTRIK && b == DOGA) return 0.5;
    if (a == SU && b == ELEKTRIK) return 0.5;
    if (a == ATES && b == SU) return 0.5;
    return 1.0;
}

// === POZİSYON & ARENA ===
enum Pozisyon { SOL, ORTA, SAG };

struct Arena {
    string ad;
    string baseMap;
    int bonusKahraman;
    int bonusDeger;
};

Arena arenalar[] = {
    {"Metropolis", R"(
+-------------------------------+
| METROPOLIS - GÖKDELENLER     |
| [L1]           [R1]           |
|                               |
+-------------------------------+
)", 4, 35},

    {"Asgard", R"(
+-------------------------------+
| ASGARD - BİFRÖST KÖPRÜSÜ    |
| [L1]           [R1]           |
|     Bifröst                   |
+-------------------------------+
)", 5, 40},

    {"Gotham", R"(
+-------------------------------+
| GOTHAM - KARANLIK ŞEHİR      |
| [L1]           [R1]           |
|     Joker Gülümsemesi         |
+-------------------------------+
)", 1, 30},

    {"Wakanda", R"(
+-------------------------------+
| WAKANDA - VİBRANİUM ORMANI   |
| [L1]           [R1]           |
|     Vibranium                 |
+-------------------------------+
)", 2, 45},

    {"Krypton", R"(
+-------------------------------+
| KRYPTON - KIZIL GÜNEŞ        |
| [L1]           [R1]           |
|     Kripton Kristali         |
+-------------------------------+
)", 4, 50},

    {"Savage Land", R"(
+-------------------------------+
| SAVAGE LAND - DİNOZOR VADİSİ |
| [L1]           [R1]           |
|     T-Rex                     |
+-------------------------------+
)", 3, 55}
};

// === YARDIMCI FONK ===
void bekle_ms(int ms) { this_thread::sleep_for(chrono::milliseconds(ms)); }
void noktaAnim(int n = 3, int ms = 250) { 
    for(int i = 0; i < n; i++) { cout << YELLOW << '.' << RESET; cout.flush(); bekle_ms(ms); } cout << endl; 
}

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string toLowerNoSpace(const string &s) {
    string t;
    for (char c : s) if (!isspace((unsigned char)c)) t += char(tolower((unsigned char)c));
    return t;
}

string safeLine() {
    string line;
    if (!getline(cin, line)) { cin.clear(); getline(cin, line); }
    return line;
}

// === ASCII EFEKTLER ===
void asciiIntro() {
    cout << BOLD << RED;
    cout << "================================================\n";
    cout << "   DEADPOOL ARENAYA GELDİ! KESİNLİKLE ÖLMEZ AI MOD!\n";
    cout << "   7 KAHRAMAN + YÜKSEK CAN + 5 ZORLUK AI + HARİTA\n";
    cout << "================================================\n" << RESET;
    bekle_ms(800);
    cout << YELLOW << "Arena hazır"; noktaAnim(5, 300);
}

// === KAHRAMAN SINIFI ===
class Kahraman {
public:
    string ad;
    int can, hasar, maxCan;
    int ozelCooldown, ultimateCooldown;
    bool stunlu;
    Element element;
    Pozisyon pozisyon;
    int toplamHasar, kritikSayisi, yenilemeSayisi, vurusSayisi, ozelKullanmaSayisi, ultimateKullanmaSayisi, selfHasar, toplamAldigiHasar;

    Kahraman() : pozisyon(ORTA) {}
    Kahraman(const string &isim, int c, int h, Element el = YOK) 
        : ad(isim), can(c), hasar(h), maxCan(c), element(el), pozisyon(ORTA) {
        ozelCooldown = ultimateCooldown = 0; stunlu = false;
        toplamHasar = kritikSayisi = yenilemeSayisi = vurusSayisi = ozelKullanmaSayisi = ultimateKullanmaSayisi = selfHasar = toplamAldigiHasar = 0;
    }

    string getRenk() {
        if (ad == "Spiderman") return MAGENTA;
        if (ad == "Batman") return BLUE;
        if (ad == "Iron Man") return RED;
        if (ad == "Hulk") return GREEN;
        if (ad == "Superman") return CYAN;
        if (ad == "Thor") return YELLOW;
        if (ad == "Deadpool") return BOLD RED;
        return WHITE;
    }

    int arenaBonus(const Arena &arena) {
        if (arena.bonusKahraman == 0 && ad == "Spiderman") return arena.bonusDeger;
        if (arena.bonusKahraman == 1 && ad == "Batman") return arena.bonusDeger;
        if (arena.bonusKahraman == 2 && ad == "Iron Man") return arena.bonusDeger;
        if (arena.bonusKahraman == 3 && ad == "Hulk") return arena.bonusDeger;
        if (arena.bonusKahraman == 4 && ad == "Superman") return arena.bonusDeger;
        if (arena.bonusKahraman == 5 && ad == "Thor") return arena.bonusDeger;
        return 0;
    }

    int mesafeBonus(Pozisyon rakipPos) {
        int mesafe = abs(static_cast<int>(pozisyon) - static_cast<int>(rakipPos));
        return (mesafe == 2) ? 15 : (mesafe == 0) ? -10 : 0;
    }

    void durumGoster() {
        int bar = (can * 20) / maxCan; if (bar < 0) bar = 0; if (bar > 20) bar = 20;
        string renk = getRenk();
        string posStr = (pozisyon == SOL) ? "SOL" : (pozisyon == SAG) ? "SAĞ" : "ORTA";
        cout << renk << BOLD << ad << " [" << posStr << "]" << RESET << " [";
        for (int i = 0; i < bar; i++) cout << "▓";
        for (int i = bar; i < 20; i++) cout << "░";
        cout << "] " << BOLD << can << "/" << maxCan << RESET << " HP";
        if (ozelCooldown > 0) cout << " | Özel:" << ozelCooldown;
        else cout << " | Özel:OK";
        if (ultimateCooldown > 0) cout << " | Ult:" << ultimateCooldown;
        else cout << " | Ult:OK";
        if (stunlu) cout << " | STUN";
        if (element != YOK) cout << " | " << elementAdi(element)[0];
        cout << endl;
    }

    void saldir(Kahraman &rakip, const string &hava, int kritikSans, const Arena &arena);
    void ozelGuc(Kahraman &rakip);
    void ultimateGuc(Kahraman &rakip, string &hava);
    void heal(const string &hava);
    void turSonuGuncelle();
    void istatistikGoster();
};

// === HARİTA GÖSTER ===
void haritaGoster(Kahraman &p1, Kahraman &p2, Arena &arena) {
    cout << "\n" << BOLD << GREEN << arena.ad << " ARENA" << RESET << endl;
    string map = arena.baseMap;

    string p1Icon = p1.getRenk() + "P1" + RESET;
    string p2Icon = p2.getRenk() + "P2" + RESET;

    size_t leftPos = map.find("[L1]");
    size_t rightPos = map.find("[R1]");

    map.replace(leftPos, 4, "    ");
    map.replace(rightPos, 4, "    ");

    if (p1.pozisyon == SOL) map.replace(leftPos, 4, p1Icon);
    else if (p1.pozisyon == SAG) map.replace(rightPos, 4, p1Icon);
    else map.replace(leftPos, 4, p1Icon);

    if (p2.pozisyon == SAG) map.replace(rightPos, 4, p2Icon);
    else if (p2.pozisyon == SOL) map.replace(leftPos, 4, p2Icon);
    else map.replace(rightPos, 4, p2Icon);

    cout << map << endl;

    int mesafe = abs(static_cast<int>(p1.pozisyon) - static_cast<int>(p2.pozisyon));
    string bonus = (mesafe == 2) ? GREEN "+15 hasar!" RESET : 
                   (mesafe == 0) ? RED "-10 hasar!" RESET : "";
    cout << YELLOW << "Mesafe: " << mesafe << " kare " << bonus << endl << RESET;
}

// === KAHRAMAN FONKSİYONLARI ===
void Kahraman::saldir(Kahraman &rakip, const string &hava, int kritikSans, const Arena &arena) {
    int baseDmg = hasar + arenaBonus(arena) + mesafeBonus(rakip.pozisyon);
    if (hava == "Gunesli") baseDmg += 20;
    else if (hava == "Sisli") baseDmg = baseDmg * 70 / 100;
    double carp = elementCarpani(element, rakip.element);
    baseDmg = int(baseDmg * carp);

    int kritik = rand() % 100;
    int dmg = baseDmg;
    if (kritik < kritikSans) {
        dmg *= 2;
        kritikSayisi++;
        cout << YELLOW << "KRİTİK VURUŞ!" << RESET << endl;
    }
    if (carp > 1.0) cout << GREEN << "(x1.5 ELEMENT GÜCÜ!)" << RESET << endl;
    else if (carp < 1.0) cout << RED << "(x0.5 ZAYIF!)" << RESET << endl;

    cout << getRenk() << ad << RESET << " → " << dmg << " hasar!\n";
    rakip.can -= dmg; if (rakip.can < 0) rakip.can = 0;
    toplamHasar += dmg; vurusSayisi++; rakip.toplamAldigiHasar += dmg;

    if (rand() % 4 == 0) {
        pozisyon = Pozisyon(rand() % 3);
        rakip.pozisyon = Pozisyon(rand() % 3);
    }
}

void Kahraman::ozelGuc(Kahraman &rakip) {
    if (ozelCooldown > 0) { cout << getRenk() << ad << " özel CD: " << ozelCooldown << RESET << "\n"; return; }
    cout << BOLD << getRenk() << ad << " ÖZEL GÜÇ!" << RESET; noktaAnim();

    int ozelDmg = hasar * 3;
    if (ad == "Hulk") ozelDmg = hasar * 5;
    if (ad == "Deadpool") ozelDmg = hasar * 4 + 50;
    double carp = elementCarpani(element, rakip.element);
    ozelDmg = int(ozelDmg * carp);

    if (ad == "Deadpool") {
        cout << BOLD RED << "BANG BANG! KATANA + TABANCA + BOMBA + TROL!" << RESET << endl;
        rakip.stunlu = true;
        can += 80; if (can > maxCan) can = maxCan;
        cout << GREEN << "Deadpool +80 HP iyileşti! (Ölümsüz!)" << RESET << endl;
    }

    rakip.can -= ozelDmg; if (rakip.can < 0) rakip.can = 0;
    if (ad != "Deadpool") rakip.stunlu = true;
    toplamHasar += ozelDmg; ozelKullanmaSayisi++; vurusSayisi++; rakip.toplamAldigiHasar += ozelDmg;
    ozelCooldown = 3;
}

void Kahraman::ultimateGuc(Kahraman &rakip, string &hava) {
    if (ultimateCooldown > 0) { cout << getRenk() << ad << " ult CD: " << ultimateCooldown << RESET << "\n"; return; }
    cout << BOLD << RED << ad << " ULTIMATE GÜÇ!" << RESET; noktaAnim(6,300);

    int dmg = 0; bool firtina = false;
    if (ad == "Deadpool") { dmg = 300; rakip.stunlu = true; cout << BOLD RED << "FOURTH WALL BREAK! 300 + STUN + TROL!" << RESET << endl; }
    else if (ad == "Hulk") { dmg = 350; can -= 100; if(can<0)can=0; selfHasar+=100; }
    else if (ad == "Thor") { dmg = 280; firtina = true; hava = "Firtinali"; }
    else dmg = 200 + rand() % 100;

    double carp = elementCarpani(element, rakip.element);
    dmg = int(dmg * carp);
    rakip.can -= dmg; if (rakip.can < 0) rakip.can = 0;

    cout << BOLD << YELLOW << dmg << " ULTIMATE HASAR!" << RESET << "\n";
    if (firtina) cout << RED << "FIRTINA BAŞLADI!\n" << RESET;
    toplamHasar += dmg; ultimateKullanmaSayisi++; vurusSayisi++; rakip.toplamAldigiHasar += dmg;
    ultimateCooldown = 4;
}

void Kahraman::heal(const string &hava) {
    if (hava == "Firtinali") { cout << "Fırtına! Heal yok!\n"; return; }
    int yenile = 60 + rand() % 41;
    if (ad == "Deadpool") yenile += 100;
    can += yenile; if (can > maxCan) can = maxCan;
    yenilemeSayisi++;
    cout << GREEN << ad << " +" << yenile << " HP (" << can << "/" << maxCan << ")\n" << RESET;
}

void Kahraman::turSonuGuncelle() {
    if (ozelCooldown > 0) ozelCooldown--;
    if (ultimateCooldown > 0) ultimateCooldown--;
    stunlu = false;
}

void Kahraman::istatistikGoster() {
    string renk = getRenk();
    cout << "\n" << renk << BOLD << ad << " İSTATİSTİK" << RESET << "\n";
    cout << "Verilen: " << toplamHasar << " | Vuruş: " << vurusSayisi << " | Kritik: " << kritikSayisi << "\n";
    cout << "İyileşme: " << yenilemeSayisi << " | Özel: " << ozelKullanmaSayisi << " | Ultimate: " << ultimateKullanmaSayisi << "\n";
    cout << "Kendine: " << selfHasar << " | Alınan: " << toplamAldigiHasar << "\n";
}

// === SEÇİM & AI ===
Arena rastgeleArena() { return arenalar[rand() % 6]; }

Kahraman karakterSec(int oyuncu, bool aiSecim = false) {
    vector<Kahraman> liste = {
        Kahraman("Spiderman", 420, 65, DOGA),
        Kahraman("Batman", 480, 55, YOK),
        Kahraman("Iron Man", 400, 85, ELEKTRIK),
        Kahraman("Hulk", 650, 90, ATES),
        Kahraman("Superman", 600, 80, ATES),
        Kahraman("Thor", 580, 95, ELEKTRIK),
        Kahraman("Deadpool", 550, 75, YOK) // ÖLÜMSÜZ!
    };

    if (aiSecim) {
        Kahraman secilen = liste[rand() % 7];
        cout << RED << "[AI] " << secilen.ad << " seçildi!\n" << RESET;
        bekle_ms(1200);
        return secilen;
    }

    while (true) {
        cout << "\n" << BOLD << "Oyuncu " << oyuncu << " KAHRAMAN SEÇ!\n" << RESET;
        for (int i = 0; i < 7; i++) {
            cout << (i+1) << ") " << liste[i].getRenk() << liste[i].ad << RESET << " (" << liste[i].maxCan << "/" << liste[i].hasar << " " << elementAdi(liste[i].element) << ")\n";
        }
        cout << "(r) Rastgele: ";
        string girdi = safeLine(); girdi = trim(girdi);
        string key = toLowerNoSpace(girdi);

        if (key == "7" || key.find("deadpool") != string::npos) return liste[6];
        if (key == "r") { Kahraman sec = liste[rand() % 7]; cout << sec.ad << " seçildi!\n"; return sec; }
        int secim = -1;
        try { secim = stoi(key) - 1; } catch(...) {}
        if (secim >= 0 && secim < 7) return liste[secim];

        cout << RED << "1-7, isim veya 'r' yaz!\n" << RESET;
    }
}

int zorlukSeviyesiSec() {
    cout << BOLD << YELLOW << "\nAI ZORLUK SEVİYESİ SEÇ!\n" << RESET;
    cout << "1) " << GREEN << "KOLAY\n2) " << CYAN << "NORMAL\n3) " << RED << "ZOR\n4) " << MAGENTA << "BRUTAL\n5) " << BOLD RED << "İMKANSIZ (TANRI)\n> " << RESET;
    int secim;
    while (true) {
        string girdi = safeLine(); trim(girdi);
        try { secim = stoi(girdi); if (secim >= 1 && secim <= 5) break; }
        catch(...) {}
        cout << RED << "1-5 yaz!\n> " << RESET;
    }
    string isim[] = {"KOLAY", "NORMAL", "ZOR", "BRUTAL", "İMKANSIZ"};
    cout << BOLD << RED << "[AI] " << isim[secim-1] << " MODU!\n" << RESET;
    bekle_ms(1500);
    return secim;
}

int kararAI(Kahraman &ai, Kahraman &rakip, const string &hava, int kritikSans, int zorluk) {
    int ol = rand() % 100;
    bool avantaj = elementCarpani(ai.element, rakip.element) > 1.0;
    bool dezavantaj = elementCarpani(ai.element, rakip.element) < 1.0;
    double canOran = (double)ai.can / ai.maxCan;

    int saldiri[6] = {0, 50, 65, 75, 85, 95};
    int ozel[6] = {0, 20, 35, 55, 70, 90};
    int ult[6] = {0, 5, 15, 30, 50, 80};
    int heal[6] = {0, 15, 30, 45, 60, 80};
    double healEsik[6] = {0, 0.6, 0.45, 0.35, 0.25, 0.15};

    if (zorluk == 5) {
        if (ai.ultimateCooldown == 0 && canOran > 0.4) return 5;
        if (ai.ozelCooldown == 0 && (avantaj || canOran < 0.5)) return 2;
        if (canOran < 0.15 && hava != "Firtinali") return 3;
        return 1;
    }

    if (canOran < healEsik[zorluk] && hava != "Firtinali" && ol < heal[zorluk]) return 3;
    if (ai.ultimateCooldown == 0 && ol < ult[zorluk]) return 5;
    if (ai.ozelCooldown == 0 && (dezavantaj || ol < ozel[zorluk])) return 2;
    if (ol < saldiri[zorluk]) return 1;
    return 4;
}

// === ANA OYUN ===
int main() {
    srand((unsigned)time(0));
    char tekrar;
    do {
        asciiIntro();

        cout << "\nMod: 1) PvP  2) vs AI\n> ";
        string mod = safeLine(); mod = trim(mod);
        bool vsAI = (mod == "2" || toLowerNoSpace(mod) == "ai");

        int aiZorluk = 3;
        if (vsAI) aiZorluk = zorlukSeviyesiSec();

        Kahraman p1 = karakterSec(1, false);
        Kahraman p2 = vsAI ? karakterSec(2, true) : karakterSec(2, false);

        Arena arena = rastgeleArena();
        cout << GREEN << "Arena: " << arena.ad << "!" << RESET << endl;

        int tur = 1, siradaki = 1;
        string hava = "Gunesli";

        while (p1.can > 0 && p2.can > 0) {
            cout << "\n" << BOLD << MAGENTA << "=== TUR " << tur << " ===" << RESET << "\n";
            hava = (rand() % 10 < 2) ? "Firtinali" : (rand() % 4 == 0 ? "Sisli" : rand() % 2 ? "Gunesli" : "Yagmurlu");
            int kritik = (hava == "Yagmurlu") ? 25 : 35;
            cout << "Hava: " << hava << " (Kritik %" << kritik << ")\n";

            haritaGoster(p1, p2, arena);
            p1.durumGoster(); p2.durumGoster();

            Kahraman *aktif = (siradaki == 1) ? &p1 : &p2;
            Kahraman *rakip = (siradaki == 1) ? &p2 : &p1;

            if (aktif->stunlu) {
                cout << RED << aktif->ad << " STUN! Pas geçiyor!\n" << RESET;
            } else {
                if (vsAI && siradaki == 2) {
                    bekle_ms(1000 + 300 * aiZorluk);
                    int secim = kararAI(*aktif, *rakip, hava, kritik, aiZorluk);
                    cout << RED << "[AI] " << aktif->ad << " hamle yapıyor" << RESET; noktaAnim(2);
                    if (secim == 1) aktif->saldir(*rakip, hava, kritik, arena);
                    else if (secim == 2) aktif->ozelGuc(*rakip);
                    else if (secim == 3) aktif->heal(hava);
                    else if (secim == 5) aktif->ultimateGuc(*rakip, hava);
                    else cout << "Pas geçti.\n";
                } else {
                    cout << "\n" << aktif->ad << " → 1=Saldır 2=Özel 3=Heal 4=Pas 5=ULT\n> ";
                    int secim = 0;
                    string girdi = safeLine();
                    try { secim = stoi(girdi); } catch(...) {}
                    if (secim == 1) aktif->saldir(*rakip, hava, kritik, arena);
                    else if (secim == 2) aktif->ozelGuc(*rakip);
                    else if (secim == 3) aktif->heal(hava);
                    else if (secim == 5) aktif->ultimateGuc(*rakip, hava);
                    else cout << "Pas geçti.\n";
                }
            }

            p1.turSonuGuncelle(); p2.turSonuGuncelle();
            cout << "\n--- TUR SONU ---\n";
            haritaGoster(p1, p2, arena);
            p1.durumGoster(); p2.durumGoster();

            siradaki = 3 - siradaki;
            if (siradaki == 1) tur++;
            bekle_ms(800);
        }

        string kazanan = (p1.can > 0 ? p1.ad : p2.ad);
        cout << "\n" << BOLD << GREEN << "KAZANAN: " << kazanan << "!" << RESET << "\n";
        p1.istatistikGoster(); p2.istatistikGoster();

        cout << "\nTekrar? (e/h): "; cin >> tekrar; cin.ignore();
    } while (tolower(tekrar) == 'e');

    cout << BOLD << CYAN << "\nARENA KAPANDI! GÖRÜŞÜRÜZ ŞAMPİYON!\n" << RESET;
    return 0;
}