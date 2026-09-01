# Agresivna eliminacija mrtvog koda (ADCE) — LLVM pass

Seminarski rad iz predmeta **Konstrukcija kompilatora**, MATF.

Pass je napisan u stilu vezbi (Legacy `FunctionPass`, `runOnFunction`, `RegisterPass`), sa sopstvenom pomocnom klasom `OurPostDominatorTree` za racunanje post-dominatora i kontrolnih zavisnosti. Dodata je i New Pass Manager registracija da bi pass radio na LLVM 18.

## Fajlovi

- `OurAggressiveDCEPass.cpp` — glavni pass (tri faze: inicijalizacija, propagacija zivosti, uklanjanje mrtvih instrukcija i grananja)
- `OurPostDominatorTree.h` / `.cpp` — sopstvena analiza post-dominacije i kontrolnih zavisnosti
- `CMakeLists.txt` — build za instaliran LLVM (apt / out-of-tree)
- `CMakeLists_intree.txt` — build za LLVM izvorni kod 
- `test/` — sest `.ll` primera

## Build (sa instaliranim LLVM 18)

    cd OurAggressiveDCE
    mkdir build && cd build
    cmake -DLT_LLVM_INSTALL_DIR=$(llvm-config-18 --prefix) ..
    make
    cd ..

Ako komanda `llvm-config-18` ne postoji, probajte samo `llvm-config`. Rezultat je `build/libOurAggressiveDCE.so`.

## Pokretanje

Nas LLVM 18 koristi New Pass Manager, pa je sintaksa `-load-pass-plugin=` i `-passes=`:

    opt -load-pass-plugin=./build/libOurAggressiveDCE.so -passes=aggressive-dead-code-elimination -S test/test1_osnovni.ll

Za druge testove promenite ime fajla na kraju. Za primere sa grananjem (test5, test6) dodajte `simplifycfg`:

    opt -load-pass-plugin=./build/libOurAggressiveDCE.so -passes='aggressive-dead-code-elimination,simplifycfg' -S test/test5_mrtvo_grananje.ll

## Šta koji test pokazuje

- `test1_osnovni` — osnovno brisanje mrtve instrukcije
- `test2_lanac` — brisanje lanca (unazad: korisnik pre operanda)
- `test3_sporedni_efekti` — instrukcije sa sporednim efektom (`store`, `call`) se ne diraju
- `test4_kontrolna_zavisnost` — grananje ostaje zivo jer vodi do zivog `store`
- `test5_mrtvo_grananje` — mrtvo grananje + PHI ograda (prepusta se simplifycfg)
- `test6_switch` — `switch` sa istim blokom kao metom vise vrednosti (demonstrira `Handled`)