% Define the symbolic variables
syms s
% Define the transfer function H(s)
Vout = -25/(2*(s^2/8 + s + 100));
Vin = 1/s;
H = Vout/Vin;
% Convert symbolic transfer function to numeric transfer function
[num, den] = numden(H); % Get numerator and denominator of H
num = sym2poly(num); % Convert symbolic numerator to polynomial
den = sym2poly(den); % Convert symbolic denominator to polynomial
H_tf = tf(num, den); % Create a numeric transfer function model
% Generate and plot the Bode plot
figure;
bode(H_tf);
grid on;
title('Bode Plot of the System');