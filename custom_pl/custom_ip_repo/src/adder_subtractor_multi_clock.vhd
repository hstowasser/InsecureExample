----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 03/09/2020 03:36:08 PM
-- Design Name: 
-- Module Name: adder_subtractor_multi_clock - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity adder_subtractor_multi_clock is
--  Port ( );
    generic(N:integer:=1024;
            DSP_Count:integer:=21); --1024/27
    port(
    reset: in std_logic;
    clk: in std_logic;
    add: in std_logic;
    A: in std_logic_vector(N+1 downto 0);
    B: in std_logic_vector(N-1 downto 0);
    Ci: in std_logic;
    Co: out std_logic;
    S: out std_logic_vector(N+1 downto 0);
    Go: in std_logic;
    Complete: out std_logic);
end adder_subtractor_multi_clock;

architecture Structural of adder_subtractor_multi_clock is

    component c_addsub_0
        port(
            A : IN STD_LOGIC_VECTOR(46 DOWNTO 0);
            B : IN STD_LOGIC_VECTOR(46 DOWNTO 0);
            CLK : IN STD_LOGIC;
            ADD : IN STD_LOGIC;
            C_IN : IN STD_LOGIC;
            C_OUT : OUT STD_LOGIC;
            S : OUT STD_LOGIC_VECTOR(46 DOWNTO 0)
        );
    end component;

    signal C:std_logic_vector(DSP_Count downto 0);
    signal Last_C:std_logic_vector(DSP_Count downto 0);
    signal Last_A: std_logic_vector(46 downto 0);
    signal Last_B: std_logic_vector(46 downto 0);
    signal Last_S: std_logic_vector(46 downto 0);
   
    --Internal counter
    signal i : integer;
begin



    --C(0) <= Ci;
    C(0) <= not add;
    
    GI: for I in 0 to N/47-1 generate
      GI:c_addsub_0 
      port map( 
          CLK=>clk,
          ADD=>add,
          A=>A(47*I+47-1 downto 47*I),
          B=>B(47*I+47-1 downto 47*I),
          C_IN=>C(I),
          C_OUT=>C(I+1),
          S=>S(47*I+47-1 downto 47*I)
          );
    end generate;
    --Last_A <= (others => '0');
    Last_A(46 downto N-DSP_Count*47+2) <= (others => '0');
    Last_A(N-DSP_Count*47-1+2 downto 0) <= A(N-1+2 downto 47*(DSP_Count) );
    --Last_B <= (others => '0');
    Last_B(46 downto N-DSP_Count*47) <= (others => '0');
    Last_B(N-DSP_Count*47-1 downto 0) <= B(N-1 downto 47*(DSP_Count) );
    
    S(N-1+2 downto DSP_Count*47) <= Last_S(N-1-DSP_Count*47+2 downto 0);
                
    G_LAST:c_addsub_0 
      port map( 
          CLK=>clk,
          ADD=>add,
          A=>Last_A,
          B=>Last_B,
          C_IN=>C(N/47),
          C_OUT=>OPEN,
          S=>Last_S
      );
      
      Co <= Last_S(37+2);
      
      
      
      -- This process block increments i up to 22. Then raises the complete flag
      process(CLK, Go, reset) -- TODO implement reset
        begin
          if reset = '0' then
            i <= 0;
            Complete <= '1';
          else
            if CLK'event and CLK='1' then
              if (Go = '1') then
                if( i > 22) then
                  Complete <= '1';
                elsif ( i > 1) then
                  if( Last_C = C ) then --In an attempt to speed this up. If there is no change in the carry bits then we are done
                      Complete <= '1';
                  end if;
                else
                  i <= i + 1;
                  Complete <= '0';
                end if;
              else
                i <= 0;
                Complete <= '0'; -- This should probably be 1 by default
              end if;
            Last_C <= C;
            end if;
          end if;
        end process;
  
  end Structural;
