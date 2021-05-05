$$ p2g5 $$
$$ Will error $$

data _z := 321 ;
data y := 567 ;

main
begin
	data y := 5 ;

	loop [ y => z ]
  		begin
  			data z := 3 ;

			begin
     			proc x ;
     		end
  		end
	;

	outter x ;
end