$$ p4g5 $$

main
begin
	data y := 5 ;
	data z := 1 ;
	data sum := 0 ;

	getter y ;

	loop [ y => z ]
    	begin
			assign sum := sum + z ;
			assign z := z + 1 ;
    	end
	;

	outter sum ;
end