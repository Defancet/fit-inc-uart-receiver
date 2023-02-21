
# Návrh číslicových systémů První část projektu – Návrh obvodu

Cílem projektu je získat základní dovednosti potřebné pro návrh a implementaci číslicových 
obvodů. Naučit se tyto obvody správně popisovat v jazyce VHDL a získat zkušenosti s jejich 
simulací a syntézou s využitím profesionálních nástrojů.

Jako zvolený příklad komponenty, na které uvedené dovednosti získáte, nám poslouží
komponenta pro příjem a vysílaní dat po asynchronní sériové lince (anglicky UART –
Universal Asynchronous Receiver-Transmitter). 

Pro jednoduchost budeme v projektu vytvářet pouze vybranou část UART řadiče, konkrétněji 
se zaměříme na přijímací část. Ta je zodpovědná za zpracování dat ze sériové linky a jejich 
rekonstrukci (deserializaci jednotlivých bitů). V porovnání s plnohodnotným řadičem UART
budeme uvažovat i řadu dalších zjednodušení, aby celková složitost vypracování projektu 
nebyla velká.

Vypracování projektu je rozděleno na dvě části. V první části jde o návrh fungování obvodu 
na úrovni RTL a také logiky jeho řídícího automatu.