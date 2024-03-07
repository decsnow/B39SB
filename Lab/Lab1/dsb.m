% Amplitude Modulation and Demodulation
% Double-Sideband Modulation Scheme
clc; clear all; close all;

fc = 40; % Carrier frequency in Hz
fm = 5; % Modulating frequency in Hz
Fs = 1000; % Sampling frequency in Hz
pe = 1.0; % phase error in radians
fe = 0.1; % frequency error in Hz

t=0:1/Fs:1;
m=cos(2*pi*fm*t)+2*cos(3*pi*fm*t); % Message signal
c=cos(2*pi*fc*t); % Carrier signal

% Modulation
x=m.*c; % Modulated signal

% Demodulation
y = x.*cos(2*pi*(fc+fe)*t+pe);
[num,den] = butter(5,2*fc/Fs); % IIR lowpass filter
y = filtfilt(num,den,y)*2; % Demodulated signal
% try butter(1,2*fc/Fs) and see what happens

% plots
subplot(221); plot (t,m)
ylabel('Amplitude'); xlabel('Time');
title('Message signal');
axis([0 1 -4 4]);
%
subplot(222); plot (t,c)
ylabel('Amplitude'); xlabel('Time');
title('Carrier signal');
axis([0 1 -1.2 1.2]);
%
subplot(223); plot (t,x)
ylabel('Amplitude'); xlabel('Time');
title('DSB modulated signal');
%
subplot(224); plot (t,y)
ylabel('Amplitude'); xlabel('Time');
title(['Demodulated signal, pe=',num2str(pe),', fe=',num2str(fe)]);
axis([0 1 -4 4]);
