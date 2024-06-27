Před spuštěním prgsem, prosím, spusťte comp_modul.
Po spuštění prgsem se objeví zpráva s návrhem vybrat parametry provádění programu.
Pak můžete ovládat průběh provádění programu pomocí kláves:
's' - set_compute
'g' - get_version
'c' - compute
'a' - abort
'q' - quit
'b' - BURST mode, tento režim můžete spustit pouze po stisknutí 'a' nebo před začátkem provádění běžného výpočtu nebo po dokončení výpočtů 
'p' - BURST mode bude prováděn nepřetržitě a bude zobrazena animace obrázku fraktálu

po stisknutí 'q' prgsem a comp_modul ukončí své provádění.

Navíc jsem napsal extra funkcionality:
Možnost výběru počtu iterací n; možnost výběru min a max komplexních čísel a možnost výběru C; možnost výběru rozměrů obrázku
BURST mode (žádám o 5 bodů), animace obrázku fraktálu(žádám o 2 body)

Poznámky:
Moje ovládací aplikace(složka prgsem) byla napsána na základě video tutoriálu profesora Jana Faigla

comp_modul ukončí práci po stisknutí 'q' v prgsem, protože po stisknutí 'q' z prgsem bude odeslána zpráva MSG_QUIT do comp_modul.
Z tohoto důvodu se v prgsem-comp_module(ref) objeví zpráva "Unknown message type has been received".

Kvůli použití SDL ve Valgrind se objeví zpráva "272 bajtů paměti je stále k dispozici".

Při použití režimu BURST, v comp_modul v Valgrind dostávám chybu "Syscall param write(buf) points to uninitialised byte(s)". Nepodařilo se mi ji opravit.

Při spuštění animace obrázku fraktálu v valgrind bude poměrně hodně chyb, i když samotná animace funguje skvěle.