clear;
SF = 7;
BW = 125000;
Fs = 125000;

s = 64; % Symbol 
SNR = 0;



gap_percent = 0.5;
gap_length = (2^SF) * gap_percent;
gap_start_position = (2^SF) * 0;
gap_end_position = gap_start_position + gap_length;


%generate a data symbol
num_samples = (2^SF) * Fs/BW;

k=s;
lora_symbol =zeros(1,num_samples);
frequnecy =zeros(1,num_samples);
for n=1:num_samples
    if k>= (2^SF)
        k= k-2^SF;
    end
    k = k+1;
    if (n>=gap_start_position) && (n<=gap_end_position)
        Amp = 0;
    else
        Amp = (1/(sqrt(2^SF)));
    end
    lora_symbol(n) = Amp*exp(1i*2*pi*(k)*(k/((2^SF)*2)));
    frequnecy(n) =k;
end
figure
subplot(2,3,1);
plot(real(lora_symbol))
subplot(2,3,2);
plot(imag(lora_symbol))
% subplot(2,3,3);
% plot(frequnecy);


%%

for j=1:1
%     lora_symbol_noisy = awgn(lora_symbol,SNR,'measured');
    lora_symbol_noisy = lora_symbol;
    
    base_down_chirp = zeros(1,num_samples); 
    k=0;
    
    for n=1:num_samples
        if k>(2^SF)
            k= k-2^SF;
        end
        k = k+1;
        base_down_chirp(n) = (1/(sqrt(2^SF)))*exp(-1i*2*pi*(k)*(k/(2^SF*2)));
    end
    
    dechirped = lora_symbol_noisy.*(base_down_chirp);

    subplot(2,3,4);
    plot(real(dechirped));
    subplot(2,3,5);
    plot(imag(dechirped));

    corrs = (abs(fft(dechirped)).^2);
    subplot(2,3,6);
    plot(corrs)

    [~,ind] = max(corrs);
    ind2(j) = ind;

    pause(0.01)
end

% histogram(ind2,2^SF)
% symbol_error_rate = sum(ind2~=s+1)/j;

 