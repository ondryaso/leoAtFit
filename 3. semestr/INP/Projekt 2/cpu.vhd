-- cpu.vhd: Simple 8-bit CPU (BrainF*ck interpreter)
-- Copyright (C) 2020 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ondrej Ondryas (xondry02@stud.fit.vutbr.cz), FIT BUT
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- ram[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_WE    : out std_logic;                    -- cteni (0) / zapis (1)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti 
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
    -- Return Address Stack --
    signal ras_top : integer range 0 to 15;
    signal ras_last : std_logic_vector(11 downto 0);
    type t_ras is array(0 to 15) of std_logic_vector(11 downto 0);
    signal ras: t_ras := (others => X"000");
    type t_rasOperation is (push, pop, none);
    signal ras_operation : t_rasOperation := none;

    -- Program Counter --
    signal pc_data : std_logic_vector(11 downto 0); -- ROM is 4 kB big, so we need 12 program addressing bits 
    type t_pcOperation is (increment, decrement, load_ras, none);
    signal pc_operation : t_pcOperation := none;

    -- RAM Pointer Register --
    signal ram_ptr_data : std_logic_vector(9 downto 0); -- RAM is 10 kB big, we need 10 data addressing bits
    type t_ramOperation is (increment, decrement, none);
    signal ram_ptr_operation : t_ramOperation := none;

    -- Loop Counter Register --
    signal lcnt_data : std_logic_vector(4 downto 0); -- We support 16 levels of loops, 5 bits are enough
    signal lcnt_operation : t_ramOperation;

    -- ALU --
    type t_aluInput is (input_data, rdata_inc, rdata_dec);
    signal alu_sel : t_aluInput := input_data;

    -- FSM --
    type fsm_state is (s_start, s_load0, s_load1, s_ram_inc, s_ram_dec, s_inc0, s_inc1, s_dec0, s_dec1,
      s_out0, s_out1, s_in0, s_in1, s_terminated, s_cycle_begin, s_cycle_ram_data, s_cycle_a, s_cycle_a_load, 
      s_cycle_b, s_cycle_c, s_cycle_end, s_cycle_end1);
    signal present_state : fsm_state := s_start;
    signal next_state : fsm_state;
begin


-- FSM sync logic --
fsmSyncLogic : process(RESET, CLK)
begin
    if (RESET = '1') then
      present_state <= s_start;
    elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
    end if;
end process fsmSyncLogic;

-- FSM next state and output logic --
fsmNextStateLogic : process(present_state, EN, OUT_BUSY, IN_VLD, CODE_DATA, DATA_RDATA, lcnt_data)
begin
  -- Default values
  ras_operation <= none;
  pc_operation <= none;
  ram_ptr_operation <= none;
  lcnt_operation <= none;
  alu_sel <= input_data;
  next_state <= present_state;
  
  CODE_EN <= '0';
  DATA_WE <= '0';
  DATA_EN <= '0';
  IN_REQ <= '0';
  OUT_WE <= '0';

  if (EN = '1') then
    case present_state is
      when s_start =>
        next_state <= s_load0;

      when s_load0 =>
        CODE_EN <= '1';
        next_state <= s_load1;
     
      when s_load1 =>
        case CODE_DATA is
          when X"3E" => -- >
            next_state <= s_ram_inc;
          when X"3C" => -- <
            next_state <= s_ram_dec;
          when X"2B" => -- +
            next_state <= s_inc0;
          when X"2D" => -- -
            next_state <= s_dec0;
          when X"5B" => -- [
            next_state <= s_cycle_begin;
          when X"5D" => -- ]
            next_state <= s_cycle_end;
          when X"2E" => -- .
            next_state <= s_out0;
          when X"2C" => -- ,
            next_state <= s_in0;
          when X"00" => -- null
            next_state <= s_terminated;
          when others =>
            pc_operation <= increment;
            next_state <= s_load0;
        end case;

      when s_ram_inc =>
        ram_ptr_operation <= increment;
        pc_operation <= increment;
        next_state <= s_load0;

      when s_ram_dec =>
        ram_ptr_operation <= decrement;
        pc_operation <= increment;
        next_state <= s_load0;

      when s_inc0 =>
        DATA_WE <= '0'; -- RAM read mode
        DATA_EN <= '1'; -- perform read
        alu_sel <= rdata_inc;
        next_state <= s_inc1;

      when s_inc1 =>
        DATA_WE <= '1'; -- RAM write mode
        DATA_EN <= '1';
        alu_sel <= rdata_inc;

        pc_operation <= increment;
        next_state <= s_load0;

      when s_dec0 =>
        DATA_WE <= '0';
        DATA_EN <= '1';
        alu_sel <= rdata_dec;
        next_state <= s_dec1;

      when s_dec1 =>
        DATA_WE <= '1'; -- RAM write mode
        DATA_EN <= '1';
        alu_sel <= rdata_dec;

        pc_operation <= increment;
        next_state <= s_load0;

      when s_out0 =>
        if (OUT_BUSY = '1') then
          next_state <= s_out0;
        else
          next_state <= s_out1;
          DATA_WE <= '0';
          DATA_EN <= '1';
        end if;

      when s_out1 =>
        OUT_WE <= '1';

        pc_operation <= increment;
        next_state <= s_load0;

      when s_in0 =>
        IN_REQ <= '1';
        next_state <= s_in1;

      when s_in1 =>
        if (IN_VLD = '1') then
          IN_REQ <= '1'; -- keep the REQ up for one more cycle to ensure getting a correct value
          alu_sel <= input_data;
          -- write to RAM
          DATA_WE <= '1';
          DATA_EN <= '1';
          
          pc_operation <= increment;
          next_state <= s_load0;
        else
          IN_REQ <= '1';
          next_state <= s_in1;
        end if;

      when s_terminated =>
        next_state <= s_terminated;

      when s_cycle_begin =>
        pc_operation <= increment;
        -- read current cell value
        DATA_WE <= '0';
        DATA_EN <= '1';

        next_state <= s_cycle_ram_data;

      when s_cycle_ram_data => -- ram[ptr] comparison
        DATA_EN <= '1';

        if (DATA_RDATA = "0000000") then
          -- cell is zero, find the corresponding ]
          -- lcnt will be == 0 here
          lcnt_operation <= increment;
          
          CODE_EN <= '1';
          next_state <= s_cycle_a_load;
        else
          -- cell is not zero, push current address and load the current instruction
          -- (we've increased PC, so we're loading the instruction following the [)
          ras_operation <= push;
          next_state <= s_load0;
        end if;

      when s_cycle_a =>
        if (lcnt_data = "00000") then
          next_state <= s_load0;
        else
          CODE_EN <= '1';
          next_state <= s_cycle_a_load;
        end if;

      when s_cycle_a_load =>
        CODE_EN <= '1';
        next_state <= s_cycle_b;

      when s_cycle_b =>
        if (CODE_DATA = X"5B") then -- [
          lcnt_operation <= increment;
        elsif (CODE_DATA = X"5D") then -- ]
          lcnt_operation <= decrement;
        end if;

        next_state <= s_cycle_c;
        pc_operation <= increment;
      
      when s_cycle_c =>
        next_state <= s_cycle_a;

      when s_cycle_end =>
        -- read current cell value
        DATA_WE <= '0';
        DATA_EN <= '1';

        next_state <= s_cycle_end1;

      when s_cycle_end1 =>
        if (DATA_RDATA = "00000000") then
          pc_operation <= increment;
          ras_operation <= pop;
          next_state <= s_load0;
        else
          pc_operation <= load_ras;
          next_state <= s_load0;
        end if;

      when others => -- sanity case
        next_state <= present_state;
    end case;
  end if;

end process;

-- Loop Counter Register --
loopCounter: process(RESET, CLK)
begin
    if (RESET = '1') then
      lcnt_data <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      case lcnt_operation is
        when increment =>
          lcnt_data <= lcnt_data + 1;
        when decrement =>
          lcnt_data <= lcnt_data - 1;
        when none =>
          lcnt_data <= lcnt_data;
      end case;
    end if;
end process loopCounter;

-- Return Address Stack --
returnAddressStack: process (RESET, CLK)
variable new_top : integer range 0 to 15;

begin
    if (RESET = '1') then
      ras <= (others => (others => '0'));
      ras_top <= 0;
      ras_last <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      ras_last <= ras(ras_top);

      case ras_operation is
        when push =>
          if (ras_top = 15) then
            new_top := 0;
          else
            new_top := ras_top + 1;
          end if;

          ras(new_top) <= pc_data;
        when pop =>
          if (ras_top = 0) then
            new_top := 15;
          else
            new_top := ras_top - 1;
          end if; 

        when none =>
          new_top := ras_top;
      end case;

      ras_top <= new_top;
    end if;
end process returnAddressStack;

-- Program Counter --
programCounter: process (RESET, CLK)
begin
    if (RESET = '1') then
      pc_data <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      case pc_operation is
        when increment =>
          pc_data <= pc_data + 1;
        when decrement =>
          pc_data <= pc_data - 1;
        when load_ras =>
          pc_data <= ras_last;
        when none => 
          pc_data <= pc_data;
      end case;
    end if;
end process programCounter;

-- RAM Pointer Register --
ramPointer: process (RESET, CLK)
begin
    if (RESET = '1') then
      ram_ptr_data <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      case ram_ptr_operation is
        when increment =>
          ram_ptr_data <= ram_ptr_data + 1;
        when decrement =>
          ram_ptr_data <= ram_ptr_data - 1;
        when none => 
          ram_ptr_data <= ram_ptr_data;
      end case;
    end if;
end process ramPointer;

-- Set entity outputs --

with alu_sel select DATA_WDATA
  <= IN_DATA        when input_data,
     DATA_RDATA + 1 when rdata_inc,
     DATA_RDATA - 1 when rdata_dec,
     "00000000"     when others;

DATA_ADDR <= ram_ptr_data;
-- DATA_WE and DATA_EN are set by FSM

OUT_DATA <= DATA_RDATA;
-- OUT_WE is set by FSM

CODE_ADDR <= pc_data;
-- CODE_EN is set by FSM

-- IN_REQ is set by FSM

end behavioral;
 
