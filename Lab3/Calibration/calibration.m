X_down = csvread('X_down.txt');
Y_down = csvread('Y_down.txt');
X_up = csvread('X_up.txt');
Y_up = csvread('Y_up.txt');
Z_up = csvread('Z_up.txt');
Z_down = csvread('Z_down.txt');

w = [X_down; Y_down; Z_down; X_up; Y_up; Z_up];

oneMat = ones(size(w,1), 1);
w = [w, oneMat];


Y = [repmat([1 0 0], size(X_down, 1), 1); repmat([0 1 0], size(Y_down, 1), 1); repmat([0 0 1], size(Z_down, 1), 1); ...
     repmat([-1 0 0], size(X_up, 1), 1); repmat([0 -1 0], size(Y_up, 1), 1); repmat([0 0 -1], size(Z_up, 1), 1)];
 
X = inv(transpose(w) * w) * transpose(w) * Y;
