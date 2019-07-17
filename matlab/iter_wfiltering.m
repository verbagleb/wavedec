%%%
% This script iterates over the following quantization parameters:
% type, size_of_block, shift_steps, 
% null_zone_extension_(layer), qf_extension_(layer)
% (See wfilering)

% Up to three layers
% Any filterbanks (set at wfiltering.m)
%%%
clear all

images(1).name = 'kiel';
images(1).common_mode = 164;
images(2).name = 'bitmap_4';
images(2).common_mode = 38;
images(3).name = 'bitmap_5';
images(3).common_mode = 251;
images(4).name = 'bitmap';
images(4).common_mode = 30; 
images(5).name = 'goldhill';
images(5).common_mode = 16;
images(6).name = 'parkrun'; 
images(6).common_mode = 73;
images(7).name = 'stockholm';
images(7).common_mode = 45;

% Requires respelling if the filterbank is not three-band
bands(1).name = 'LL';
bands(2).name = 'LM';
bands(3).name = 'LH';
bands(4).name = 'ML';
bands(5).name = 'MM';
bands(6).name = 'MH';
bands(7).name = 'HL';
bands(8).name = 'HM';
bands(9).name = 'HH';

for i = 1:9
    bands(i).W = floor((i-1)/3)+1;  
    bands(i).H = mod(i-1,3)+1;
end

% Measure time performance
tictoc = true;
% Take plots form the folder_mask dir and draw upon them
load_plots = false;
% Save plots in the folder_mask dir
save_plots = true;
% Save cropped plots as well (parameters adjusted above)
save_cropped = false;
% Drawing ratio of values to original ones (just dwt with no modifications)
relative = false;

% Plot dir
folder_mask = '../plots/';

% Null zone extension for the main investigated band (MIV) (read below)
null_zone_extension = 1.0;  
% Skip all operations over MIV: MIV considered to be undistorted
% (not contibuting into PSNR and not taken into account in entropy)
% DEFAULT: false
skip_main = false;
% Number of quantizing steps (!! qf = 1/quantizing_step !!)
qf_n = 10;
% Qf bounds
qf_best = 0.3;
qf_worst = 0.01;

qf_list = 10.^(linspace(log10(qf_best), log10(qf_worst),qf_n));
% qf_list = [2.00000 0.70000 0.20000 0.10000 0.03000 0.01500 0.00600 0.00300 0.00070 0.00030];

% Current figure index
fig_num=0;
% From here LL band is designated as "Main investigated band" (MIV)
% The other ones are taken in coules by layers
% If you don't solve specific problems, let it be bands(1) (LL)
band = bands(1);
for im_num = 1 % or other image indices
    image = images(im_num);
    folder = [folder_mask];
    % Folder containing text files with images
    im_name = ['../files/' image.name '/fileSub_Y_0_orig.txt'];
    im_common_mode = image.common_mode;
%         figname = [folder image.name '_' num2str(size_of_block) 'x' num2str(size_of_block) '.fig'];
%         bmpname = [folder image.name '_' num2str(size_of_block) 'x' num2str(size_of_block) '.bmp'];
    % Names for .fig, .bmp and cropped .bmp (if used) plots accordingly
    figname = [folder image.name '.fig'];
    bmpname = [folder image.name '.bmp'];
    cropname = [folder image.name '_crop.bmp'];
    % Name for the plot
    titlelabel = [image.name ' - Complex, profit'];
    % Read a plot
    if load_plots
        openfig(figname);
    % Or create a new one
    else
        fig_num = fig_num+1;
        figure(fig_num);
        cla;
    end
    hold on;
    % Recommended to fix if automatic plot saving is supposed
    set(gcf,'Position', [200 200 1025 650]);
    bandW = band.W;
    bandH = band.H;

    %%% See wfiltering to find the following global variables
    % Line type for plots
    line_type = '-';
    % DWT depth
    layers = 3;

    % Quantization type. 1 is DEFAULT type
    type = 1;
    % Just the string for legend output
    idea = '0';
    % Size of block in quantizing (ignored in modes 1, 2)
    size_of_block = 5;
    % Number of candidate shift values relatively to quantization step
    % (ignored in modes 1,2)
    shift_steps = 3;
    % Extensions of the null zone of the quantization grid of this layer
    % relatively to the quantization step of this layer (DEFAULT 1.0)
    null_zone_extension_1 = 1.0;
    null_zone_extension_2 = 1.0;
    null_zone_extension_3 = 1.0;
    % Extensions of quantization step relatively to the one of the MIV
    % by layer (DEFAULT 1.0)
    qf_extension_1 = 1.0;
    qf_extension_2 = 1.0;
    qf_extension_3 = 1.0;
    % THE OPERATION ITSELF
    % Result: entropys and psnrs
    wfiltering;
    % Storing the obtained values as "original" (nessary for relative mode)
    entropys_0 = entropys;
    psnrs_0 = psnrs;

    % Some other parameters (necessary for relative mode)

    i_plot = 0;

    i_plot = i_plot + 1;
    type = 5;
    idea = '1';
    size_of_block = 5;
    shift_steps = 3;
    null_zone_extension_1 = 1.5;
    null_zone_extension_2 = 1.5;
    null_zone_extension_3 = 1.0;
    qf_extension_1 = 1.0;
    qf_extension_2 = 1.0;
    qf_extension_3 = 1.0;
    wfiltering;
    if relative
        % Storing strings of the modificated modes
        entropys_i(:,i_plot) = entropys;
        psnrs_i(:,i_plot) = psnrs;
    end

    % Etc for other parameter sets, don't forget i_plot++
    % ...

    % Relative mode: ratio of the method entropy to the original one at
    % the same PSNR
    if relative  

        % Interpolation is needed to get "the same PSNR"
        % Number of interpolation points
        n_p=150; % should be not less that qf_n

        p_m = max(min([psnrs_0,psnrs_i]));
        p_M = min(max([psnrs_0,psnrs_i]));
        p_v = linspace(p_m,p_M,n_p).';
        e_0 = interp1(psnrs_0,entropys_0,p_v,'pchip');
        clear e;
        % Makes as many strings as there were "modificated modes"
        for i_plot = 1:size(psnrs_i,2)
            e(:,i_plot) = interp1(psnrs_i(:,i_plot),entropys_i(:,i_plot),p_v,'pchip');
        end

        % Plot the plots
        plot(p_v,e./e_0*100);
        % Line of 100%
        if ~load_plots
            plot(p_v,100*ones(1,n_p),'k--');
        end

        legend({'Равномерное квантование',
            'Сдвиг НЧ 3 уровня на медиану в блоках 5x5, x1.5 нуль-зона ВЧ 1 и 2 уровня', 
            'Найденный оптимум'})
        xlabel('PSNR, dB');
        ylabel('Relative entropy, %');
        title(titlelabel);

    % Absolute mode: original and modes are plotted seperately
    else      

        xlabel('Энтропия, биты');
        ylabel('PSNR, dB');
        title(titlelabel);
    end
    % End of absolute mode

    shg;

    if save_plots 
        savefig(figname);
        saveas(gcf, bmpname);
        % Example of cropping 1/3 range of PSNR
        if save_cropped && ~relative
            yl=ylim;
            ylc = mean(yl);
            yld = diff(yl);
            c=1/3;
            ylim(ylc+[-1 1]*yld*c/2);
            saveas(gcf, cropname);
        end
    end


end