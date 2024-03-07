% Quadrature Amplitude Modulation
clc; clear all; close all;

fc = 40; % Carrier frequency in Hz
fm1 = 2; % Modulating frequency in Hz
fm2 = 5 % Modulating frequency in Hz
Fs = 1000; % Sampling frequency in Hz

t=0:1/Fs:1;
m1=cos(2*pi*fm1*t)+2*cos(3*pi*fm1*t); % Message signal 1
m2=cos(2*pi*fm2*t)+2*cos(5*pi*fm2*t); % Message signal 2

c1=cos(2*pi*fc*t); % In-phase carrier signal
c2=sin(2*pi*fc*t); % Quadrature-phase carrier signal

% Modulation
x1=m1.*c1; % Modulated signal 1
x2=m2.*c2; % Modulated signal 2
x=x1+x2;

%demodulation
y1 = x1 .* c1
y2 = x2 .* c2
m1_demodulated = lowpass(y1, fm1, Fs);
m2_demodulated = lowpass(y2, fm2, Fs);

% plots
figure(1),
subplot(221); plot (t,m1)
ylabel('Amplitude'); xlabel('Time');
title('Message signal 1');
%
subplot(222); plot (t,m2)
ylabel('Amplitude'); xlabel('Time');
title('Message signal 2');
%
subplot(223); plot (t,c1)
ylabel('Amplitude'); xlabel('Time');
title('Carrier signal');
%
subplot(224); plot (t,x)
ylabel('Amplitude'); xlabel('Time');
title('QAM signal');
%

figure(2),
subplot(211); plot(t, m1_demodulated);
title('Demodulated Message Signal 1');
xlabel('Time'); ylabel('Amplitude');
%
subplot(212); plot(t, m2_demodulated);
title('Demodulated Message Signal 2');
xlabel('Time'); ylabel('Amplitude');