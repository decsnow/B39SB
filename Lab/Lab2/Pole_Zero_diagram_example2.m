% Plot pole-zero diagram of a Laplace transform
% Example X(s)=1/(s+1)
num=[1 9 20];
den=[1 5 17 13];
zs=roots(num)
ps=roots(den)
pzmap(ps,zs)
