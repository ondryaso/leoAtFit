-- Author: Ond?ej Ondryá? (xondry02@stud.fit.vutbr.cz)
-- Date: 2020-10-30

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

-- Vigenere Cipher interface
entity vigenere is
   port(
        -- Input signals
        CLK : in std_logic;
        RST : in std_logic;
        DATA : in std_logic_vector(7 downto 0);
        KEY : in std_logic_vector(7 downto 0);

        -- Output signals
        CODE : out std_logic_vector(7 downto 0)
    );
end vigenere;

architecture behavioral of vigenere is
    type t_fsmState is (addShift, subtractShift);
    type t_fsmOutput is (pos, neg, err);

    signal shift : std_logic_vector(7 downto 0);
    signal corPositiveShifted : std_logic_vector(7 downto 0);
    signal corNegativeShifted : std_logic_vector(7 downto 0);

    signal presentState : t_fsmState;
    signal nextState : t_fsmState;
    signal forceErr : std_logic;

    signal fsmOutput : t_fsmOutput;
begin

    shiftCalculator: process(KEY)
    begin
        -- 'A' has ASCII value 65; when 'A' is the value of KEY, the shift should be 1
        shift <= KEY - 64;
    end process shiftCalculator;

    positiveShift : process(DATA, shift)
    variable sum : std_logic_vector(7 downto 0);
    begin 
        sum := DATA + shift;

        if (sum > 90) then
            -- There are 26 A-Z letters
            -- This wouldn't correct for any value higher than 116 (using modulo would solve that)
            -- but it is not required by the assignment
            sum := sum - 26;
        end if;

        corPositiveShifted <= sum;
    end process positiveShift;

    negativeShift : process(DATA, shift)
    variable sum : std_logic_vector(7 downto 0);
    begin 
        sum := DATA - shift;

        if (sum < 65) then
            sum := sum + 26;
        end if;

        corNegativeShifted <= sum;
    end process negativeShift;

    fsmSyncLogic : process(RST, CLK)
    begin
        forceErr <= '0';

        if (RST = '1') then
            presentState <= addShift;
            forceErr <= '1';
        elsif (CLK'event AND CLK = '1') then
            presentState <= nextState;
        end if;
    end process fsmSyncLogic;

    -- Next state and output logic for the FSM.
    fsmNextStateLogic : process(presentState, forceErr, DATA)
    begin
        case presentState is
            when addShift =>
                nextState <= subtractShift;

                if (forceErr = '1') then
                    fsmOutput <= err;
                else
                    fsmOutput <= pos;
                end if;
            when subtractShift =>
                nextState <= addShift;
                fsmOutput <= neg;
            when others =>
                nextState <= subtractShift;
                fsmOutput <= pos;
        end case;

        if (DATA > 47 AND DATA < 58) then
            fsmOutput <= err; -- DATA is a numeric character, put '#' to output
        end if;
    end process fsmNextStateLogic;

    -- Output multiplexor
    CODE <= corPositiveShifted when (fsmOutput = pos) else
            corNegativeShifted when (fsmOutput = neg) else
            conv_std_logic_vector(35, 8); -- '#' has ASCII value 35, convert to 8 bits

end behavioral;
