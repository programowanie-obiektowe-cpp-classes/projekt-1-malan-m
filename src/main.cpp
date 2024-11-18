#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <random>
#include <deque>
#include "RandomNameGenerator.hpp"
#include "RandomWydzialGenerator.hpp"

// --- Stale wydajnosci ---

constexpr double CI = 50.0;     // Wartość produktu zwiększana przez inżyniera
constexpr double CMag = 100.0;  // Pojemność magazynu zwiększana przez magazyniera
constexpr double CMkt = 10.0;   // Popyt na produkt zwiększany przez marketera
constexpr double CR = 5.0;      // Liczba produktów tworzonych przez robotnika
constexpr int MAX_KREDYT_RATY = 60; // Maksymalna liczba rat kredytu
constexpr int N=3; // liczba miesiecy uzywana do obliczania wartosci firmy
constexpr double M = 3.0; // Maksymalny stosunek zadłużenia do wartości firmy
constexpr double POCZATKOWA_KWOTA = 100000.0; // Stała początkowa kwota na koncie


// --- Klasa Pracownik oraz jej pochodne ---

class Pracownik {
public:
    std::string imie;
    virtual void print() const = 0;
    virtual ~Pracownik() = default;
};

class Inz : public Pracownik {
public:
    std::string wydzial;
    static constexpr double wynagrodzenie = 2000.0;
    void print() const override {
        std::cout << "Inzynier: " << imie << ", Wydzial: " << wydzial << "\n";
    }
};

class Mag : public Pracownik {
public:
    bool obsl_widl;
    static constexpr double wynagrodzenie = 1800.0;
    void print() const override {
        std::cout << "Magazynier: " << imie << ", Obsluga wozka widlowego: " 
                  << (obsl_widl ? "Tak" : "Nie") << "\n";
    }
};

class Mkt : public Pracownik {
public:
    int followers;
    static constexpr double wynagrodzenie = 2500.0;
    void print() const override {
        std::cout << "Marketer: " << imie << ", Obserwujacy: " << followers << "\n";
    }
};

class Rob : public Pracownik {
public:
    double but;
    static constexpr double wynagrodzenie = 1500.0;
    void print() const override {
        std::cout << "Robotnik: " << imie << ", Rozmiar buta: " << but << "\n";
    }
};

// --- Klasa Kredyt ---

class Kredyt {
public:
    double dlug;
    int pozostale_raty;
    double rata;

    Kredyt(double kwota, int czas_splaty) 
        : dlug(kwota), pozostale_raty(czas_splaty) {
        rata = dlug / pozostale_raty;
    }

    double splac_rate() {
        if (pozostale_raty > 0) {
            pozostale_raty--;
            return rata;
        }
        return 0.0;
    }
};

// --- Klasa Firma ---

class Firma {

private:
    double stan_konta;
    std::vector<std::unique_ptr<Kredyt>> kredyty;
    std::vector<std::variant<Inz, Mag, Mkt, Rob>> pracownicy;
    double historia_przychodu = 0.0;
    std::deque<double> historia_przychodow; // Ostatnie przychody (maks. N elementów)

public:

Firma() {
    //Stan konta poczatkowy
     stan_konta = POCZATKOWA_KWOTA;

    //Inzunier
    Inz inzynier;
    inzynier.imie = getRandomName();        
    inzynier.wydzial = getRandomWydzial();       
    zatrudnij(inzynier);

    //Magazynier
    Mag magazynier;
    magazynier.imie = getRandomName();
    magazynier.obsl_widl = rand() % 2;
    zatrudnij(magazynier);

    //Marketer
    Mkt marketer;
    marketer.imie = getRandomName();       
    marketer.followers = rand() % 10000;
    zatrudnij(marketer);

    // Robotnik
    Rob robotnik;
    robotnik.imie = getRandomName();
    robotnik.but = 40 + (rand() % 20)/5;
    zatrudnij(robotnik);

    //Tekst poczatkowy    
    std::cout << "Witaj w grze.\nTwoim zadaniem bedzie rozwoj firmy do wartosci 50 000zl\n";
    std::cout << "\nNa poczatek na koncie znajduje sie: "<<  POCZATKOWA_KWOTA <<"zl";
    std::cout << "\n\nFirma posiada po jednym pracowniku kazdego rodzaju: \n"; drukuj_pracownikow();
    std::cout << "\nKompendy:\n    lp - lista pracownikow\n    zinz - zatrudnij inzyniera\n    zmmag - zatrudnij magazyniera\n    zmkt - zatrudnij marketera\n    zrob - zatrudnij robotnika\n";

}

    void zatrudnij(std::variant<Inz, Mag, Mkt, Rob> pracownik) {
        pracownicy.push_back(std::move(pracownik));
    }

void wez_kredyt(double kwota, int czas_splaty) {
    if (czas_splaty > MAX_KREDYT_RATY) {
        std::cout << "Maksymalny okres spłaty to " << MAX_KREDYT_RATY << " miesięcy.\n";
        return;
    }

    double oprocentowanie = czas_splaty * 0.02; // 2% za każdą ratę
    double calkowita_kwota_do_splaty = kwota * (1.0 + oprocentowanie);

    // Sprawdzenie, czy nowe zadłużenie nie przekroczy limitu M razy wartość firmy
    double laczne_zadluzenie = calkowita_kwota_do_splaty;
    for (const auto& kredyt : kredyty) {
        laczne_zadluzenie += kredyt->dlug;
    }

    double maks_zadluzenie = M * get_wartosc_firmy();

    if (laczne_zadluzenie > maks_zadluzenie) {
        std::cout << "Nie mozna zaciagnac kredytu: przekroczono maksymalne zadluzenie (" 
                  << maks_zadluzenie << ").\n";
        return;
    }

    // Dodanie kredytu do listy
    kredyty.push_back(std::make_unique<Kredyt>(calkowita_kwota_do_splaty, czas_splaty));
    stan_konta += kwota;

    std::cout << "Kredyt na kwote " << kwota << " zaciagnięty na " << czas_splaty 
              << " miesiecy. Oprocentowanie calkowite: " << oprocentowanie * 100 << "%.\n";
}

void zaplac_wynagrodzenie() {
    double suma_wynagrodzen = 0;
    for (auto& pracownik : pracownicy) {
        std::visit([&](auto& p) {
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Inz>)
                suma_wynagrodzen += Inz::wynagrodzenie;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Mag>)
                suma_wynagrodzen += Mag::wynagrodzenie;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Mkt>)
                suma_wynagrodzen += Mkt::wynagrodzenie;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Rob>)
                suma_wynagrodzen += Rob::wynagrodzenie;
        }, pracownik);
    }
    stan_konta -= suma_wynagrodzen;
}

    void splac_raty() {
        for (auto& kredyt : kredyty) {
            stan_konta -= kredyt->splac_rate();
        }
    }

    void oblicz_przychod() {
    int liczba_inz = 0, liczba_mag = 0, liczba_mkt = 0, liczba_rob = 0;
    double suma_wynagrodzen = 0;

    for (const auto& pracownik : pracownicy) {
        std::visit([&](const auto& p) {
            suma_wynagrodzen += p.wynagrodzenie;  // sumowanie wynagrodzen kazdego pracownika
            if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Inz>) liczba_inz++;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Mag>) liczba_mag++;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Mkt>) liczba_mkt++;
            else if constexpr (std::is_same_v<std::decay_t<decltype(p)>, Rob>) liczba_rob++;
        }, pracownik);
    }

    double pojemnosc_magazynu = liczba_mag * CMag;
    double cena_produktu = liczba_inz * CI;
    double popyt = liczba_mkt * CMkt;
    double max_produkcja = liczba_rob * CR;
    double rzeczywista_produkcja = std::min(pojemnosc_magazynu, max_produkcja);
    double sprzedane_produkty = std::min(popyt, rzeczywista_produkcja);

    double przychod = sprzedane_produkty * cena_produktu;
    stan_konta += (przychod - suma_wynagrodzen);
    historia_przychodow.push_back(przychod);
    if (historia_przychodow.size() > N) {
        historia_przychodow.pop_front(); // Usuwanie najstarszego elementu
    }
}


    double get_stan_konta() const {
        return stan_konta;
    }

    double get_wartosc_firmy() const {
        return historia_przychodu;
    }

    void drukuj_pracownikow() const {
        for (const auto& pracownik : pracownicy) {
            std::visit([](const auto& p) { p.print(); }, pracownik);
        }
    }

    double get_wartosc_firmy()  {
    if (historia_przychodow.empty()) return 0.0; // Gdy wartosc firmy jest pusta, to jej wartosc jest rwona 0
    double suma = 0.0;
    for (double przychod : historia_przychodow) {
        suma += przychod;
    }
    return suma / historia_przychodow.size();
}
};

// --- Klasa Gra ---

class Gra {
    Firma firma;
    bool stan = true;

public:
    void akcja_gracza() {
        std::string akcja;
        std::cout << "Podaj komende: ";
        std::cin >> akcja;

        if (akcja == "lp") {
            firma.drukuj_pracownikow();
        } else if (akcja == "zinz") {
            Inz inz;
            inz.imie = getRandomName();
            inz.wydzial= getRandomWydzial();
            firma.zatrudnij(inz);
        } else if (akcja == "zmag") {
            Mag mag;
            mag.imie = getRandomName();
            mag.obsl_widl = rand() % 2;
            firma.zatrudnij(mag);
        } else if (akcja == "zmkt") {
            Mkt mkt;
            mkt.imie = getRandomName();
            mkt.followers = rand() % 10000;
            firma.zatrudnij(mkt);
        } else if (akcja == "zrob") {
            Rob rob;
            rob.imie = getRandomName();
            rob.but = 40 + (rand() % 20)/10; //beda rozmiary z polowkami
            firma.zatrudnij(rob);
        } else if (akcja == "kred") {
            double kwota;
            int czas;
            std::cout << "Podaj kwote kredytu: ";
            std::cin >> kwota;
            std::cout << "Podaj czas splacania w miesiacach: ";
            std::cin >> czas;
            firma.wez_kredyt(kwota, czas);
        } else if (akcja == "kt") {
            firma.oblicz_przychod();
            firma.zaplac_wynagrodzenie();
            firma.splac_raty();

            std::cout << "Stan konta: " << firma.get_stan_konta() << "\n";
            std::cout << "Wartosc firmy: " << firma.get_wartosc_firmy() << "\n";

            if (firma.get_wartosc_firmy() > 50000.0) {
                std::cout << "Gratulacje! Twoja firma odniosla sukces!\n";
                stan = false;
            } else if (firma.get_stan_konta() < 0) {
                std::cout << "Bankructwo! Gra skonczona.\n";
                stan = false;
            }
        }
    }

    bool get_stan() const {
        return stan;
    }
};

int main() {
    Gra gra;
    while (gra.get_stan()) {
        gra.akcja_gracza();
    }
    return 0;
}