% Laplace transform 
% Example x(t)=5u(t)-exp(-2t)u(t)
syms t s
x=5*heaviside(t)-exp(-2*t)*heaviside(t);
X=laplace(x,t,s);
pretty(X);

x1 = (t.^5)*heaviside(t);
X1 =laplace(x1,t,s);
pretty(X1);

x2 = -2*heaviside(t)+4*t*exp(-2*t)*heaviside(t)+7*exp(-4*t)*heaviside(t);
X2 =laplace(x2,t,s);
pretty(X2);

x3 = exp(-3*t)*cos(5*t)*heaviside(t);
X3 =laplace(x3,t,s);
pretty(X3);