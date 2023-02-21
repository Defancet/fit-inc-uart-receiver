-- uart.vhd: UART controller - receiving part
-- Author(s): xkalut00 Maksim Kalutski
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------
entity UART_RX is
port(	
	CLK		: in std_logic;
	RST		: in std_logic;
	DIN		: in std_logic;
	DOUT     : out std_logic_vector(7 downto 0);
	DOUT_VLD : out std_logic
);
end UART_RX;  

-------------------------------------------------
architecture behavioral of UART_RX is
signal clk_cnt   : std_logic_vector(4 downto 0);
signal bit_cnt   : std_logic_vector(3 downto 0);
signal rx_en     : std_logic;
signal cnt_en    : std_logic;
signal dout_vald : std_logic;
begin
	FSM: entity work.UART_FSM(behavioral)
		port map (
			CLK      => CLK,
			RST 	   => RST,
			DIN    	=> DIN,
			CLK_CNT  => clk_cnt,
			BIT_CNT  => bit_cnt,
			RX_EN    => rx_en,
			CNT_EN 	=> cnt_en,
			DOUT_VLD => dout_vald
		);
	DOUT_VLD <= dout_vald;
	process(CLK) begin
		if (RST = '1') then
			DOUT <= "00000000";
		end if;
		if rising_edge(CLK) then
			if cnt_en = '1' then
				clk_cnt <= clk_cnt + 1;
			else 
				clk_cnt <= "00000";
				bit_cnt <= "0000";
			end if;
			if rx_en = '1' then		
				if clk_cnt(4) = '1' then
					clk_cnt <= "00001";
					case bit_cnt is
					when "0000" => DOUT(0) <= DIN;
					when "0001" => DOUT(1) <= DIN;
					when "0010" => DOUT(2) <= DIN;
					when "0011" => DOUT(3) <= DIN;
					when "0100" => DOUT(4) <= DIN;
					when "0101" => DOUT(5) <= DIN;
					when "0110" => DOUT(6) <= DIN;
					when "0111" => DOUT(7) <= DIN; 
					when others => null;
					end case;
					bit_cnt <= bit_cnt + 1;
				end if;
			end if;
		end if;
	end process;
end behavioral;