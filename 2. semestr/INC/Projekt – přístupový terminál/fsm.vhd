-- fsm.vhd: Finite State Machine
-- Author(s): Ondrej Ondryas (xondry02@stud.fit.vutbr.cz)
--
library ieee;
use ieee.std_logic_1164.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- Input signals
   KEY         : in  std_logic_vector(15 downto 0);
   CNT_OF      : in  std_logic;

   -- Output signals
   FSM_CNT_CE  : out std_logic;
   FSM_MX_MEM  : out std_logic;
   FSM_MX_LCD  : out std_logic;
   FSM_LCD_WR  : out std_logic;
   FSM_LCD_CLR : out std_logic
);
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is 
   type t_state is (
   DIG0, DIG1, DIG2A, DIG2B, DIG3A, DIG3B, 
	DIG4A, DIG4B, DIG5A, DIG5B, DIG6A, DIG6B,  
	DIG7A, DIG7B, DIG8A, DIG8B, DIG9A, DIG9B,
	START,
   WRONG,
   PRINT_FAIL,
   PRINT_SUC,
   FINISH);
	
	signal present_state, next_state : t_state;
begin
-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= START;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, KEY, CNT_OF)
begin
   case (present_state) is
   
   when START =>
      next_state <= START;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(2) = '1') then
         next_state <= DIG0;
      end if;
      end if;

   when DIG0 =>
      next_state <= DIG0;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(0) = '1') then
         next_state <= DIG1;
      end if;		 
      end if;
		  
	when DIG1 =>
      next_state <= DIG1;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(5) = '1') then
         next_state <= DIG2A;
      end if;	

      if (KEY(9) = '1') then
         next_state <= DIG2B;
      end if;		  
      end if;

   when DIG2A =>
      next_state <= DIG2A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(1) = '1') then
         next_state <= DIG3A;
      end if;		 
      end if;
		  
   when DIG2B =>
      next_state <= DIG2B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
        elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(3) = '1') then
         next_state <= DIG3B;
      end if;		 
      end if;

   when DIG3A =>
      next_state <= DIG3A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(6) = '1') then
         next_state <= DIG4A;
      end if;		 
      end if;

   when DIG3B =>
      next_state <= DIG3B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(6) = '1') then
         next_state <= DIG4B;
      end if;		 
      end if;
		
	when DIG4A =>
      next_state <= DIG4A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(9) = '1') then
         next_state <= DIG5A;
      end if;		 
      end if;

   when DIG4B =>
      next_state <= DIG4B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(4) = '1') then
         next_state <= DIG5B;
      end if;		 
      end if;	

   when DIG5A =>
      next_state <= DIG5A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(5) = '1') then
         next_state <= DIG6A;
      end if;		 
      end if;
		  
   when DIG5B =>
      next_state <= DIG5B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(4) = '1') then
         next_state <= DIG6B;
      end if;		 
      end if;
		  
	when DIG6A =>
      next_state <= DIG6A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(3) = '1') then
         next_state <= DIG7A;
      end if;		 
      end if;
		  
   when DIG6B =>
      next_state <= DIG6B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(5) = '1') then
         next_state <= DIG7B;
      end if;		 
      end if;
		  
	when DIG7A =>
      next_state <= DIG7A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(5) = '1') then
         next_state <= DIG8A;
      end if;		 
      end if;
		  
   when DIG7B =>
      next_state <= DIG7B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(8) = '1') then
         next_state <= DIG8B;
      end if;		 
      end if;
		  
	when DIG8A =>
      next_state <= DIG8A;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(7) = '1') then
         next_state <= DIG9A;
      end if;		 
      end if;
		  
   when DIG8B =>
      next_state <= DIG8B;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;
        
      if (KEY(4) = '1') then
         next_state <= DIG9B;
      end if;		 
      end if;
		  
   when DIG9A =>
      next_state <= DIG9A;
      if KEY(15) = '1' then
         next_state <= PRINT_SUC;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG; 
      end if;
		  
   when DIG9B =>
      next_state <= DIG9B;
      if KEY(15) = '1' then
         next_state <= PRINT_SUC;
      elsif KEY(14 downto 0) /= "000000000000000" then
         next_state <= WRONG;	 
      end if;

   when WRONG =>
      next_state <= WRONG;
      if KEY(15) = '1' then
         next_state <= PRINT_FAIL;
      end if;
   when PRINT_SUC =>
      next_state <= PRINT_SUC;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   when PRINT_FAIL =>
      next_state <= PRINT_FAIL;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   when FINISH =>
		next_state <= FINISH;
		if KEY(15) = '1' then
			next_state <= START;
		end if;
   when others =>
      next_state <= START;
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, KEY)
begin
   FSM_CNT_CE     <= '0';
   FSM_MX_MEM     <= '0';
   FSM_MX_LCD     <= '0';
   FSM_LCD_WR     <= '0';
   FSM_LCD_CLR    <= '0';

   case (present_state) is
	when PRINT_FAIL =>
	   FSM_CNT_CE     <= '1';
		FSM_MX_MEM		<= '0';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';

	when PRINT_SUC =>
		FSM_CNT_CE     <= '1';
      FSM_MX_MEM		<= '1';
      FSM_MX_LCD     <= '1';	
      FSM_LCD_WR     <= '1';

   when FINISH =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;

   when others =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
		elsif (KEY(14 downto 0) /= "000000000000000") then
         FSM_LCD_WR     <= '1';
      end if;
   end case;
	
end process output_logic;

end architecture behavioral;

