% Inverse laplace transform
% Example X(s)=(s-5)/(s(s+3)^2)
syms t s
% X=(s-5)/(s*(s+3)^2)


X1=(s^2+2*s+1)/((s+1)*(s^2+5*s+6));
x1=ilaplace(X1);
pretty(x1);

X2=(s^2+1)/(s*(s^2+2*s+17));
x2=ilaplace(X2);
pretty(x2);
