clear;
format long;

SF = 7;
BW = 125000; % Hz
Fs = 1e7;

symbol_duration = (1/BW)*(2^SF);
symbol_value = 88;
symbol_start_frequency = BW/(2^SF)*symbol_value; % Hz

bandwidth_max_frequency = BW;
bandwidth_min_frequency = 0;

t_part_1 = ((2^SF)-symbol_value)*1/BW;
t_part_2 = symbol_duration - t_part_1;

samples_upchirp_part_1 = 0:1/Fs:t_part_1;
samples_upchirp_part_2 = t_part_1:1/Fs:symbol_duration;

chirp_part_1 = chirp(samples_upchirp_part_1,symbol_start_frequency,t_part_1,bandwidth_max_frequency);
chirp_part_2 = chirp(samples_upchirp_part_2,bandwidth_min_frequency,symbol_duration,symbol_start_frequency);
subplot(2,3,1)
plot(samples_upchirp_part_1,chirp_part_1);
subplot(2,3,2)
plot(samples_upchirp_part_2,chirp_part_2);

lora_chirp = [chirp_part_1, chirp_part_2];
lora_chirp(end) =[];
subplot(2,3,3)
plot(lora_chirp);

samples_downchirp = 0:1/Fs:symbol_duration;
base_down_chirp = chirp(samples_downchirp,bandwidth_max_frequency,symbol_duration,bandwidth_min_frequency);

subplot(2,3,4)
plot(samples_downchirp,base_down_chirp);


%%

dechirped = fft(lora_chirp).*fft(base_down_chirp);
corrs = (abs(fft(dechirped)).^2);
figure
plot(corrs);
[~,ind] = max(corrs);




