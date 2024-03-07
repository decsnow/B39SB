% Frequency and phase modulation
clc; clear all; close all;

fc = 400; % carrier frequency in Hz
fm = 25; % modulating frequency in Hz
kf = 150;
kp = 8;

t=0:0.0001:0.1;
m=cos(2*pi*fm*t); % Message signal 
c=cos(2*pi*fc*t); % Carrier signal

% PM signal
x_PM = cos(2*pi*fc*t+kp*m);
% FM signal
x_FM = cos(2*pi*fc*t+kf*cumsum(m)*0.001); 

% plots
figure(1),
subplot(221); plot (t,m)
ylabel('Amplitude'); xlabel('Time');
title('Message signal');
axis([0 0.1 -1.2 1.2])
%
subplot(223); plot (t,c)
ylabel('Amplitude'); xlabel('Time');
title('Carrier signal');
axis([0 0.1 -1.2 1.2])
%
subplot(222); plot (t,x_PM)
ylabel('Amplitude'); xlabel('Time');
title('Phase modulated signal');
axis([0 0.1 -1.2 1.2])
%
subplot(224); plot (t,x_FM)
ylabel('Amplitude'); xlabel('Time');
title('Frequency modulated signal');
axis([0 0.1 -1.2 1.2])

