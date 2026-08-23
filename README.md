# NeuroVoice

NeuroVoice este un proiect care ajută persoanele care nu pot vorbi să răspundă cu DA sau NU folosind semnale EEG.

Pe ecran apar două zone care pâlpâie: DA la 10 Hz și NU la 15 Hz. Persoana privește răspunsul dorit, iar ESP32 analizează semnalul primit prin ADS1115. Interfața folosește răspunsurile pentru a forma și rosti un mesaj, de exemplu: „Am nevoie de apă”.

Componentele folosite sunt ESP32, ADS1115, un amplificator EEG și electrozi.

