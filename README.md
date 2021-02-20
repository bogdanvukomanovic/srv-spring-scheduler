# Sistemi u realnom vremenu

## Spring raspoređivanje

Napisati raspoređivač nad FreeRTOS-om koji omogućava raspoređivanje taskova po Spring algoritmu. Sistem treba da obezbedi detaljno logovanje izvršavanja. Davanje ulaza sistemu treba da bude moguće kroz konfiguracionu datoteku, kao i sa konzole. Neophodno je implementirati Spring raspoređivač nad FreeRTOS-om - kao na vežbama 5 i 6, tj. bez izmene samog operativnog sistema. Raspoređivač se implementira kao task visokog prioriteta koji raspoređuje ostale taskove nad postojećim FreeRTOS raspoređivačem.

Spring algoritam treba da podržava:

1. Sve osnovne heurističke funkcije:
    - a - vreme pristizanja - FCFS (First Come First Served) 
    - C - vreme izračunavanja (vreme potrebno procesoru da izvrši task bez prekidanja) - SJF (Shortest Job First)
    - d - vreme pre koga task mora da se završi da ne bi nastale bilo kakve posledice po sistem - EDF (Earliest Deadline First)
    - d + W * C
		
2. Rad sa resursima:
    - T<sub>est</sub>
    - d + W * T<sub>est</sub>
		
3. Ograničenje prethođenja:
    - E
		
Poslovi se zadaju sistemu na izvršavanje u grupama (batch). Spring raspoređivač će pri startovanju svake grupe kao parametar primiti niz heurističkih funkcija koje treba da koristi pri raspoređivanju. Ako sistem može da rasporedi grupu po nekoj funkciji, on to i učini, a u suprotnom prijavljuje da to nije moguće. Raspored nije dobar ako bilo koji posao probija svoj rok.
Ako raspored jeste izvodiv, sistem treba da izvrši poslove u tom redosledu, i da u tekstualnoj datoteci zapiše svaku zamenu konteksta, tj. u kojem tačno tick-u je započet koji od poslova. 
Zadavanje ulaza

Pri zadavanju ulaza, korisnik treba da može da navede sledeće elemente:

1. Resursi:
    - Nabrojati koji resursi postoje u sistemu, i za svaki od njih koje je najranije vreme kada može da se koristi.
		
2. Podaci o poslovima:
    
    - Startno vreme.
    - Vreme izvršavanja - pretpostavljamo da korisnik zna za svaki task tačno koliko on traje, kao i da neće pogrešiti pri unosu.
    - Deadline.
    - Funkcija posla, data kao stringovno ime zadatka - pretpostavka je da su funkcije fiksirane u sistemu i da se neće dinamički menjati.
    - Niz resursa koje ovaj posao koristi.
		
3. Izbor niza heurističkih funkcija koje će biti korišćene za izvršavanje.
	
4. Ograničenje prethođenja.
	
Ovaj ulaz treba da može u potpunosti da se zada kroz tekstualnu datoteku ili sa konzole. Nema potrebe potržati kombinaciju ova dva tipa unosa. Sistem treba da podržava izvršavanje jedne konfiguracije poslova, ne više njih. Svi vremenski parametri se zadaju u tick-ovima.
