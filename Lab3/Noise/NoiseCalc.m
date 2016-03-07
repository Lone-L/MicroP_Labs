z_d_n = csvread('Zero_degree_measurement_noise.txt');

Vr = var(z_d_n);
Mr = mean(z_d_n);

z_d_p_n = csvread('Zero_degree_process_noise.txt');
Vq = var(z_d_p_n) - Vr;
Mq = mean(z_d_p_n);

test = csvread('test5.txt');
