-- uart_fsm.vhd: UART controller - finite state machine
-- Author(s): xkalut00 Maksim Kalutski
--
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------
entity UART_FSM is
port(
   CLK      : in std_logic;
   RST	   : in std_logic;
	DIN      : in std_logic;
   CLK_CNT 	: in std_logic_vector(4 downto 0);
   BIT_CNT  : in std_logic_vector(3 downto 0);
   RX_EN    : out std_logic;
   CNT_EN   : out std_logic;
   DOUT_VLD : out std_logic
   );
end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
type STATE_TYPE is (START, FIRST_BIT, RECEIVE_DATA, LAST_BIT, VALIDATE);
signal state : STATE_TYPE := START;
begin
RX_EN    <= '1' when state = RECEIVE_DATA else '0';
CNT_EN   <= '1' when state = FIRST_BIT or state = RECEIVE_DATA else '0';
DOUT_VLD <= '1' when state = VALIDATE else '0';
   process (CLK) begin
       if rising_edge(CLK) then
         if RST = '1' then
            state <= START; 
         else
            case state is
            when START        => if DIN = '0' then 
												state <= FIRST_BIT;
                                 end if;
            when FIRST_BIT => if CLK_CNT = "10000" then
												state <= RECEIVE_DATA;
                                 end if;
				when RECEIVE_DATA => if BIT_CNT = "1000" then
												state <= LAST_BIT;
                                 end if;
            when LAST_BIT     => if DIN = '1' then
												state <= VALIDATE;
                                 end if;
            when VALIDATE     => state <= START;
            when others       => null;
            end case;             
         end if;
      end if;
   end process;
end behavioral;