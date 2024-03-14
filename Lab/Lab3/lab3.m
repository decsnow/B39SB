clear;
clc;

% Define the given values
R1 = 1000; % Resistance in ohms
R2 = 200; % Resistance in ohms
C1 = 0.125e-3; % Capacitance in farads
C2 = 50e-6; % Capacitance in farads

% Define the transfer function for Vout(s) using the correct derivation
numerator = -C1*R1;
denominator = [C1*C2*R1*R2 C2*R2 1]; % Corrected to make the TF proper
Vout = tf(numerator, denominator); % Create a transfer function object

% Display the transfer function for Vout(s)
disp('The transfer function for Vout(s) is:');
Vout
damp(Vout);

% (ii) Find out vout(t) using Matlab
syms s t;
sn = poly2sym(numerator,s);
sd = poly2sym(denominator,s);
vout = ilaplace(sn/sd)*heaviside(t);
vout

% (iii) Using Matlab to plot vin(t) and vout(t) on the same graph
V_in = 1/s;
vin_t = ilaplace(V_in)*heaviside(t);
t = [-0.5,5]; % Time vector from  to 5 seconds
% vin_t = heaviside(t); % Step input remains 1 after t=0
figure;
fplot(vin_t,t); % Plot both vin and vout
hold on;
fplot(vout,t);
xlabel('Time (s)');
ylabel('Amplitude');
title('Input and Output Voltages Over Time');
legend('v_{in}(t)', 'v_{out}(t)');
grid on;



% (iv) Plot the impulse response of the system
Vout=-25/(2*(s^2/8 + s + 100));
Vin=1/s;
H=Vout/Vin;
h=ilaplace(H);
t = [0,5];
figure;
fplot(h,t);
title('Impulse response of the System');


% (v) Obtain the Bode plots
[num, den] = numden(H); % Get numerator and denominator of H
num = sym2poly(num); % Convert symbolic numerator to polynomial
den = sym2poly(den); % Convert symbolic denominator to polynomial
H_tf = tf(num, den); % Create a numeric transfer function model
% Generate and plot the Bode plot
figure;
bode(H_tf);
grid on;
title('Bode Plot of the System');

