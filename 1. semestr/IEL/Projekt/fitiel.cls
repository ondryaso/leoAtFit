%%% Nepovinná šablona pro projekty IEL %%%
%%% Verze: 2019/01 %%%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{fitiel}[2019/10/27 IEL project class]

% základní třída, odvozujeme ze třídy article
\LoadClass[a4paper,11pt]{article}

% nastavení parametrů
\newif\ifczech \let\ifczech\iftrue
\newif\ifslovak \let\ifslovak\iffalse
\newif\ifenglish \let\ifenglish\iffalse

\DeclareOption{english}{\let\ifenglish\iftrue\let\ifczech\iffalse}
\DeclareOption{slovak}{\let\ifslovak\iftrue\let\ifczech\iffalse}
\DeclareOption*{}
\ProcessOptions\relax

% odstranění čísla částí (sections) z nadpisů
\renewcommand{\@seccntformat}[1]{}

%% základní balíčky - šablona
\RequirePackage{ifthen}

%% základní balíčky - dokument
\RequirePackage[left=2cm,text={17cm, 24cm},top=3cm]{geometry}
\RequirePackage{graphicx}
\RequirePackage{float}

% utf-8
\RequirePackage[T1]{fontenc}
\RequirePackage[utf8]{inputenc}

% matematika
\RequirePackage{amsmath}

% klikatelné odkazy (sekce v obsahu)
\RequirePackage[unicode]{hyperref}

% kreslení obvodů 
%\usepackage[european,straightvoltages,betterproportions,EFvoltages]{circuitikz}

% jednotky
\RequirePackage{siunitx}
\sisetup{locale = DE,detect-weight=true,detect-family=true,detect-inline-family=math,output-complex-root=\ensuremath{\mathrm{j}}}
%%

%% logo a nastavení správného jazyka pro dokument
% čeština
\ifczech
    \usepackage[czech]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_cz.png}
    }
\else
\fi

% slovenština
\ifslovak
    \usepackage[slovak]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_cz.png}
    }
\else
\fi

% angličtina
\ifenglish
    \usepackage[english]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_en.png}
    }
\else
\fi

% rozšířená matice
\makeatletter
\renewcommand*\env@matrix[1][*\c@MaxMatrixCols c]{%
  \hskip -\arraycolsep
  \let\@ifnextchar\new@ifnextchar
  \array{#1}}
\makeatother

\AtEndOfClass{
    % použité značky 
    \newcommand{\Vo}{\si \volt}
    \newcommand{\Am}{\si \ampere}
    \newcommand{\uF}{\si {\micro\farad}}
    \newcommand{\mH}{\si {\milli\henry}}
    \newcommand{\Hz}{\si \hertz}
    
    % prvni priklad

    \newcommand{\prvniZadani}[1]{
        \def\prvniSkupina{#1}
        Stanovte napětí $U_{R5}$ a proud $I_{R5}$. Použijte metodu postupného zjednodušování obvodu. \\
        
        \begin{tabular}{|c|c|c|c|c|c|c|c|c|c|c|} \hline
sk. & $U_1$ [\Vo] & $U_2$ [\Vo]  & $R_1$ [$\Omega$] & $R_2$ [$\Omega$] & $R_3$ [$\Omega$] & $R_4$ [$\Omega$] & $R_5$ [$\Omega$] & $R_6$ [$\Omega$] & $R_7$ [$\Omega$] & $R_8$ [$\Omega$] \\ \hline
    \ifthenelse{\equal{#1}{A}}
    {A & 80 & 120 & 350 & 650 & 410 & 130 & 360 & 750 & 310 & 190}{}
    \ifthenelse{\equal{#1}{B}}
    {B & 95 & 115 & 650 & 730 & 340 & 330 & 410 & 830 & 340 & 220}{}
    \ifthenelse{\equal{#1}{C}}
    {C & 100 & 80 & 450 & 810 & 190 & 220 & 220 & 720 & 260 & 180}{}
    \ifthenelse{\equal{#1}{D}}
    {D & 105 & 85 & 420 & 980 & 330 & 280 & 310 & 710 & 240 & 200}{}
    \ifthenelse{\equal{#1}{E}}
    {E & 115 & 55 & 485 & 660 & 100 & 340 & 575 & 815 & 255 & 225}{}
    \ifthenelse{\equal{#1}{F}}
    {F & 125 & 65 & 510 & 500 & 550 & 250 & 300 & 800 & 330 & 250}{}
    \ifthenelse{\equal{#1}{G}}
    {G & 130 & 60 & 380 & 420 & 330 & 440 & 450 & 650 & 410 & 275}{}
    \ifthenelse{\equal{#1}{H}}
    {H & 135 & 80 & 680 & 600 & 260 & 310 & 575 & 870 & 355 & 265}{}
    \\ \hline \end{tabular} \\
    
    \begin{center}
     \includegraphics[scale=0.5,keepaspectratio]{fig/Pr1_2019.pdf}
    \end{center}
    }
        
    % druhy priklad
    \newcommand{\druhyZadani}[1]{
        \def\druhySkupina{#1}
        Stanovte napětí $U_{R6}$ a proud $I_{R6}$. Použijte metodu Théveninovy věty. \\
        
         \begin{tabular}{|c|c|c|c|c|c|c|c|c|} \hline
sk. & $U$ [\Vo] & $R_1$ [$\Omega$] & $R_2$ [$\Omega$] & $R_3$ [$\Omega$] & $R_4$ [$\Omega$] & $R_5$ [$\Omega$] & $R_6$ [$\Omega$] \\ \hline
    \ifthenelse{\equal{#1}{A}}
    {A & 50 & 100 & 525 & 620 & 210 & 530 & 100}{}
    \ifthenelse{\equal{#1}{B}}
    {B & 100 & 50 & 310 & 610 & 220 &  570 & 200}{}
    \ifthenelse{\equal{#1}{C}}
    {C & 200 & 70 & 220 & 630 & 240 & 450 & 300}{}
    \ifthenelse{\equal{#1}{D}}
    {D & 150 & 200 & 200 & 660 & 200 & 550 & 400}{}
    \ifthenelse{\equal{#1}{E}}
    {E & 250 & 150 & 335 & 625 & 245 & 600 & 150}{}
    \ifthenelse{\equal{#1}{F}}
    {F & 130 & 180 &350 & 600 & 195 & 650 & 250}{}
    \ifthenelse{\equal{#1}{G}}
    {G & 180 & 250 & 315 & 615 & 180 & 460 & 350}{}
    \ifthenelse{\equal{#1}{H}}
    {H & 220 & 190 & 360 & 580 & 205 & 560 & 180}{}
    \\ \hline \end{tabular} \\
    
    \begin{center}
     \includegraphics[scale=0.5,keepaspectratio]{fig/Pr2_2019.pdf}
    \end{center}
    }

    % treti priklad
    \newcommand{\tretiZadani}[1]{
        \def\tretiSkupina{#1}
        Stanovte napětí $U_{R4}$ a proud $I_{R4}$. Použijte metodu uzlových napětí ($U_A$, $U_B$, $U_C$). 
        
        \begin{tabular}{|c|c|c|c|c|c|c|c|c|} \hline
sk. & $U$ [\Vo] & $I_1$ [\Am] & $I_2$ [\Am] & $R_1$ [$\Omega$] & $R_2$ [$\Omega$] & $R_3$ [$\Omega$] & $R_4$ [$\Omega$] & $R_5$ [$\Omega$] \\ \hline
    \ifthenelse{\equal{#1}{A}}
    {A & 120 & 0.9  & 0.7  & 53 & 49 & 65 & 39 & 32}{}
    \ifthenelse{\equal{#1}{B}}
    {B & 150 & 0.7  & 0.8  & 49 & 45 & 61 & 34 & 34}{}
    \ifthenelse{\equal{#1}{C}}
    {C & 110 & 0.85 & 0.75 & 44 & 31 & 56 & 20 & 30}{}
    \ifthenelse{\equal{#1}{D}}
    {D & 115 & 0.6  & 0.9  & 50 & 38 & 48 & 37 & 28}{}
    \ifthenelse{\equal{#1}{E}}
    {E & 135 & 0.55 & 0.65 & 52 & 42 & 52 & 42 & 21}{}
    \ifthenelse{\equal{#1}{F}}
    {F & 145 & 0.75 & 0.85 & 48 & 44 & 53 & 36 & 25}{}
    \ifthenelse{\equal{#1}{G}}
    {G & 160 & 0.65 & 0.45 & 46 & 41 & 53 & 33 & 29}{}
    \ifthenelse{\equal{#1}{H}}
    {H & 130 & 0.95 & 0.50 & 47 & 39 & 58 & 28 & 25}{}
    \\ \hline \end{tabular} \\
    
    \begin{center}
     \includegraphics[scale=0.5,keepaspectratio]{fig/Pr3_2019.pdf}
    \end{center}
    }
    
    % ctvrty  priklad
    \newcommand{\ctvrtyZadani}[1]{
        \def\ctvrtySkupina{#1}
        Pro napájecí napětí platí: $u_1=U_1{\cdot}\sin(2{\pi}ft)$, $u_2=U_2{\cdot}\sin(2{\pi}ft)$.\\
Ve vztahu pro napětí $u_{C_{2}}=U_{C_2}{\cdot}\sin(2{\pi}ft+\varphi_{C_{2}})$
určete $|U_{C_{2}}|$ a $\varphi_{C_{2}}$. Použijte metodu smyčkových proudů.\\

Pozn: Pomocné směry šipek napájecích zdrojů platí pro speciální časový okamžik ($t=\frac{\pi}{2{\omega}}$).
        
        \begin{tabular}{|c|c|c|c|c|c|c|c|c|c|} \hline
sk. & $U_1$ [\Vo] & $U_2$ [\Vo] & $R_1$ [$\Omega$] & $R_2$ [$\Omega$] & $L_{1}$ [\mH] & $L_{2}$ [\mH] & $C_1$ [$\uF$] & $C_2$ [$\uF$] & $f$ [\Hz] \\ \hline
    \ifthenelse{\equal{#1}{A}}
    {A & 35 & 55 & 12 & 14 & 120 & 100 & 200 & 105 & 70}{}
    \ifthenelse{\equal{#1}{B}}
    {B & 25 & 40 & 11 & 15 & 100 &  85 & 220 &  95 & 80}{}
    \ifthenelse{\equal{#1}{C}}
    {C & 35 & 45 & 10 & 13 & 220 &  70 & 230 &  85 & 75}{}
    \ifthenelse{\equal{#1}{D}}
    {D & 45 & 50 & 13 & 15 & 180 &  90 & 210 &  75 & 85}{}
    \ifthenelse{\equal{#1}{E}}
    {E & 50 & 30 & 14 & 13 & 130 &  60 & 100 &  65 & 90}{}
    \ifthenelse{\equal{#1}{F}}
    {F & 20 & 35 & 12 & 10 & 170 &  80 & 150 &  90 & 65}{}
    \ifthenelse{\equal{#1}{G}}
    {G & 55 & 50 & 13 & 12 & 140 &  60 & 160 &  80 & 60}{}
    \ifthenelse{\equal{#1}{H}}
    {H & 65 & 60 & 10 & 10 & 160 &  75 & 155 &  70 & 95}{}
    \\ \hline \end{tabular} \\
    
    \begin{center}
     \includegraphics[scale=0.5,keepaspectratio]{fig/Pr4_2019.pdf}
    \end{center}
    }

    % paty priklad
    \newcommand{\patyZadani}[1]{
        \def\patySkupina{#1}
        V obvodu na obrázku níže v čase $t=0 $[s] sepne spínač $S$. Sestavte diferenciální rovnici popisující chování obvodu na obrázku, dále ji upravte dosazením hodnot parametrů. Vypočítejte analytické řešení
$u_{C}~=~f(t)$. Proveďte kontrolu výpočtu dosazením do sestavené diferenciální rovnice.\\

Pozn: Pomocné směry šipek napájecích zdrojů platí pro speciální časový okamžik ($t=\frac{\pi}{2{\omega}}$).\\
        
        \begin{tabular}{|c|c|c|c|c|} \hline
        sk. & $U$ [\Vo] & $C$ [F] & $R$ [$\Omega$] & $u_C(0)$ [\Vo] \\ \hline
    \ifthenelse{\equal{#1}{A}}
    {A & 40 & 50 & 10 & 16}{}
    \ifthenelse{\equal{#1}{B}}
    {B & 30 & 10 & 20 & 15}{}
    \ifthenelse{\equal{#1}{C}}
    {C & 35 & 5 & 30 & 14}{}
    \ifthenelse{\equal{#1}{D}}
    {D & 25 & 5 & 25 & 12}{}
    \ifthenelse{\equal{#1}{E}}
    {E & 40 & 30 & 40 & 11}{}
    \ifthenelse{\equal{#1}{F}}
    {F & 22 & 30 & 15 & 10}{}
    \ifthenelse{\equal{#1}{G}}
    {G & 20 & 50 & 25 & 8}{}
    \ifthenelse{\equal{#1}{H}}
    {H & 18 & 50 & 40 & 5}{}
    \\ \hline \end{tabular} \\
    
    \begin{center}
     \includegraphics[scale=0.5,keepaspectratio]{fig/Pr5_2019.pdf}
    \end{center}
    }
    
}