syms s t Y 
% example y'''+8y'' + 19y'' + 12y = t**heaviside(t), y(0)=1, y'(0)=0, y''(0)=0
x=t*heaviside(t);
X=laplace(x,t,s);
Y1=s*Y-1;
% Y1 is Laplace transform of first derivative, Y1=sY(s)-y(0);
Y2 = s*Y1;
% Y2 is Laplace transform of second derivative, Y2=sY1-y'(0);
Y3 = s*Y2;
% Y3 is Laplace transform of third derivative, Y3=sY2-y''(0);
Sol = solve(Y3+8*Y2+19*Y1+12*Y-X, Y);
sol=ilaplace(Sol,s,t);
% sol is y(t)
pretty(sol)
figure
fplot(sol, [0 10])
% Change the interval of t (must be from 0 to a positive value)
% as necessary for your problem 
