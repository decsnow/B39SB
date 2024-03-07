syms s t Y 
% example y'' + 6y'' + 8y = t*exp(-3*t)*heaviside(t), y(0)=1, y'(0)=1
x=t*exp(-3*t)*heaviside(t);
X=laplace(x,t,s);
Y1=s*Y-1;
% Y1 is Laplace transform of first derivative, Y1=sY(s)-y(0);
Y2 = s*Y1-1;
% Y2 is Laplace transform of second derivative, Y2=sY1-y'(0);
Sol = solve(Y2+6*Y1+8*Y-X, Y);
sol=ilaplace(Sol,s,t);
% sol is y(t)
pretty(sol)
figure
fplot(sol, [0 10])
% Change the interval of t (must be from 0 to a positive value)
% as necessary for your problem 
