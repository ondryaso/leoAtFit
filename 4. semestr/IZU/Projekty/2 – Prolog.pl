concatenated([], L, L).
concatenated([H|T1], L2, [H|T3]) :- concatenated(T1, L2, T3).
rotate([H|T], X) :- concatenated(T, [H], X), !.

uloha16([A|B], [], [A|B]). % Prázdný klíč -> vstupní seznamy se musí rovnat
uloha16([], [_|_], []). % Ukončovací klauzule -> v seznamech nic nezbývá

% Verze 1: X je striktně vstupní seznam, tj. Prolog naváže uloha16([...], [...], Y), ale ne uloha16(X, [...], [...])
uloha16([X|InputRem], [K|KeyRem], [Y|OutputRem]) :- Y is (X - K), rotate([K|KeyRem], KRotated), uloha16(InputRem, KRotated, OutputRem).

% Verze 2: Prolog zvládne navázat proměnné v obou směrech
% uloha16([X|InputRem], [K|KeyRem], [Y|OutputRem]) :- ground(X), !, Y is (X - K), rotate([K|KeyRem], KRotated), uloha16(InputRem, KRotated, OutputRem).
% uloha16([X|InputRem], [K|KeyRem], [Y|OutputRem]) :- X is (Y + K), rotate([K|KeyRem], KRotated), uloha16(InputRem, KRotated, OutputRem).