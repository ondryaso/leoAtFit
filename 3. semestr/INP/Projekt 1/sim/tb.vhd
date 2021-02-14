library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity testbench is
end testbench;

architecture behavioral of testbench is
constant period : time := 1 ns;

component vigenere
    port(
      CLK : in std_logic;
      RST : in std_logic;
      DATA : in std_logic_vector(7 downto 0);
      KEY : in std_logic_vector(7 downto 0);

      CODE : out std_logic_vector(7 downto 0)
    );
end component;

signal char_sig: character;

type tLOGIN is array(0 to 7) of character;
type tKEY is array(0 to 1) of character;

-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-- Sem doplnte svuj login:
constant login_signal: tLOGIN := (
    'X', 'O', 'N', 'D', 'R', 'Y', '0', '2'
);
-- Sem doplnte klic (prvni dve pismena prijmeni bez diakritiky):
constant key_signal: tKEY := (
    'O', 'N'
);
-- !!!!!!!!! Ostatni casti tohoto kodu nemodifikujte !!!!!!!!!!!

signal clk : std_logic := '0';
signal rst : std_logic := '1';
signal tb_data : std_logic_vector(7 downto 0);
signal tb_key : std_logic_vector(7 downto 0);
signal tb_code : std_logic_vector(7 downto 0);

begin

    uut : vigenere
    port map(
      CLK => clk,
      RST => rst,
      DATA => tb_data,
      KEY => tb_key,

      CODE => tb_code
    );

    clk <= NOT clk AFTER period / 2;

    test : process
    begin
        -- inicializace vstupu (jen aby na zacatku nebyly 'X')
        tb_key <= CONV_STD_LOGIC_VECTOR(character'pos(key_signal(0)), 8);
        tb_data <= CONV_STD_LOGIC_VECTOR(character'pos(login_signal(0)), 8);
        wait until clk'event AND clk = '1'; -- reset (vyse inicializovan na '1')
        rst <= '0'; -- shodime reset
        for i in 0 to 7 loop -- a cyklime pres znaky loginu a klice...
            tb_key <= CONV_STD_LOGIC_VECTOR(character'pos(key_signal(i mod 2)), 8);
            tb_data <= CONV_STD_LOGIC_VECTOR(character'pos(login_signal(i)), 8);
            wait until clk'event AND clk = '1';
        end loop;
        wait;
    end process;

end behavioral;
