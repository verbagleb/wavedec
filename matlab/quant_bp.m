Nbands = 9;
immask = 'Graphs/Distributions/Shift/%s/%s_%s_b%d_%f.%s';
% immask = 'Graphs/Distributions/error/%s/error_norm_%s_%s_b%d_%f.%s';
% for bitmap_name = {'bitmap', 'bitmap\_4', 'bitmap\_5', 'kiel', 'goldhill', 'parkrun', 'stockholm'}
for bitmap_name = {'bitmap\_4'}
suffix = '';
width=[]; % [] for auto
bitmap_name_full= [erase(bitmap_name{1},'\') suffix];
delta = 368.332995/128; %the response of LL to shift
shift = 0:255;
% for qf = [0.1 0.03 0.01 0.005 0.0015 0.0005]
for qf = [0.3]
for comp = {'Y'} %'Y'/'Cb'/'Cr'
    n=1;
    fname = ['files/0/', bitmap_name_full, '/fileSub_', comp{1}, '_', num2str(n-1), '_orig.txt'];
    component = flip(importdata(fname),1);
    component_col = component(:);
    component_shifted = component_col - shift*delta;
    component_quant = round(component_shifted*qf);
    component_error = component_shifted-component_quant/qf;
    return;
    
    ent = zeros(size(shift));
    for i=1:numel(shift)
        ent(i)=entropy(component_quant(:,i));
    end
    PSNR = 20*log10(256./rms(component_error,1));
    
%     figure(1);
    subplot(2,2,1);
    hold off;
%     set(gcf,'Position', [500 250 925 600]);
%     plot(shift, (ent-mean(ent))/std(ent,1));
    plot(shift, ent);
    title(['shift-bitrate - ', bitmap_name{1}, ' - band ', num2str(n)]);
    xlabel('Shift');
    ylabel('Entropy, bits');
    xlim([0 255]);
    
%     figure(2);
    subplot(2,2,3);
%     set(gcf,'Position', [500 250 925 600]);
    hold off;
    plot(shift, PSNR);
    title(['shift-PSNR - ', bitmap_name{1}, ' - band ', num2str(n)]);
    xlabel('Shift');
    ylabel('PSNR, dB');
    xlim([0 255]);

%     figure(3);
    subplot(1,2,2);
%     set(gcf,'Position', [500 250 925 600]);
    hold off;
    plot(ent, PSNR);
    title(['bitrate-PSNR - ', bitmap_name{1}, ' - band ', num2str(n)]);
    xlabel('Entropy, bits');
    ylabel('PSNR, dB');

%     imname = sprintf(immask, 'fig', bitmap_name_full, comp{1}, n, qf, 'fig');
%     savefig(imname);
%     imname = sprintf(immask, 'bmp', bitmap_name_full, comp{1}, n, qf, 'bmp');
%     saveas(gcf, imname);
end
end
end