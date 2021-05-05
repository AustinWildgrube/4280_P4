$$
File: fib.fs21
Date: April 2, 2021
Author: Jared Diehl
Description: Prints the Fibonacci sequence up to a user-specified limit
$$

data _MIN := 1; $$ Minimum allowed _N $$
data _MAX := 99; $$ Maximum allowed _N $$
data _N := 0; $$ Number of terms in sequence $$
data a := 0; $$ x(n-2) $$
data b := 1; $$ x(n-1) $$
data sum := 0; $$ x(n) $$

data input := 0;
data fib := 0;

main
begin
    $$ Loop until we have the number of terms $$
    void input;
    getter _N;
    if [_N => _MIN - 1] then
        proc input;
    ;
    if [_N =< _MAX + 1] then
        proc input;
    ;

    $$ Print first _N terms of Fibonacci sequence $$
    void fib;
    if [_N [==] 0] then
        begin
            $$ Print term $$
            outter a;

            $$ Create next term $$
            assign sum := a + b;
            assign a := b;
            assign b := sum;

            $$ Decrement _N $$
            assign _N := _N - 1;

            $$ Jump to fib $$
            proc fib;
        end
    ;
end