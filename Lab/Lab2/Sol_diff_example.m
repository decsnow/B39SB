syms s t Y 
% example y'' + 4y'' + 2y = exp(-t), y(0)=4, y'(0)=5
x=exp(-t);
X=laplace(x,t,s)
Y1=s*Y-4;
% Y1 is Laplace transform of first derivative, Y1=sY(s)-y(0);
Y2 = s*Y1-5;
% Y2 is Laplace transform of second derivative, Y2=sY1-y'(0);
Sol = solve(Y2+4*Y1+2*Y-X, Y)
sol=ilaplace(Sol,s,t)
% sol is y(t)
pretty(sol)
figure
fplot(sol, [0 15])
% Change the interval of t (must be from 0 to a positive value)
% as necessary for your problem 
