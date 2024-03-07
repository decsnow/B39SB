% Plot pole-zero diagram of a Laplace transform
% Example X(s)=1/(s+1)
num=[1 5];
den=[1 2 4];
zs=roots(num)
ps=roots(den)
pzmap(ps,zs)
