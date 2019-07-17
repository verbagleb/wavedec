%%%
% One dwt filtering loop
% Result: entropys and psnrs, and a plot as well (in non-relaitve mode)
% Any number of bands, any filterbanks
% Up tp 3 layers
%%%

% Parameters are set from beyond the script? (e.g., using iter_wfiltering):
external = false;

if ~external 
    
    clear all; % Use if you launch wfiltering directly
    
    tictoc = true; % Time perfection measurement
    idea = 'Simple';
    relative = false;

    im_name = '../files/kiel/fileSub_Y_0_orig.txt';

    bandH = 1;
    bandW = 1;
    skip_main = false;
    layers = 3;
    
    qf_n = 50; % Number of quantizing steps ( !! qf=1/quant_step !! )
    qf_best = 0.3;  % Bounds
    qf_worst = 0.01;
    qf_list = 10.^(linspace(log10(qf_best), log10(qf_worst),qf_n));
    
    type = 1;
    shift_steps = 0;
    size_of_block = 20;
    null_zone_extension = 1.0;
    null_zone_extension_1 = 1.0;
    null_zone_extension_2 = 1.0;
    null_zone_extension_3 = 1.0;
    qf_extension_1 = 1.0;
    qf_extension_2 = 1.0;
    qf_extension_3 = 1.0;
    
    line_type = '-';
end

if tictoc
    disp(idea);
    tic
end

% Halfs of direct (a) and inverse (b) filterbanks
a__= [  12 0.7172150000	0.4821190000	0.1007850000	-0.0572617000	-0.0434293000	-0.0025706500	0.0076218200	0.0016123000	0.0009144900	0.0001070570	-0.0004118280	0.0000812236
        12 0.0000000000	-0.6879050000	0.0528450000	0.1928270000	-0.0239495000	0.0337432000	-0.0031931300	-0.0082939700	0.0008158960	-0.0000712195	0.0002739660	-0.0000540336
        12 0.6853030000	-0.4999470000	0.1461390000	0.0322635000	-0.0249847000	0.0092418300	-0.0046033700	-0.0002193460	-0.0006710620	-0.0000620485	0.0002386880	-0.0000470757];
b__=[   7 0.6636790000	0.4996830000	0.1553180000	-0.0494516000	-0.0573942000	-0.0081058900	0.0123629000
        7 0.0000000000	0.6880630000	0.0473184000	-0.1537010000	-0.0260899000	-0.0047508900	0.0015184900
        7 0.6624620000	-0.5099850000	0.0954987000	0.0783513000	-0.0690810000	-0.0085326700	0.0195878000];

% If no analysis is performed, there is one band
if layers>0
    N=3;
else
    N=1;
end
% Symmetrical (1) of anti-symmetrical (-1) filter expansion
sym_coef = [1 -1 1];
dir_filter=[a__(:,end:-1:3).*sym_coef.', a__(:,2:end)];
inv_filter=[b__(:,end:-1:3).*sym_coef.', b__(:,2:end)];
% Normalizing
inv_norm = sqrt(sum(inv_filter.^2,2));
dir_filter=dir_filter.*inv_norm;
inv_filter=inv_filter./inv_norm;

% The response of LL to prior shift
delta = sum(dir_filter(1,:))^2; 

% Image taken from test file
original=importdata(im_name);

% layers = 0;
[h_or, w_or]=size(original);

% DWT layer by layer
clear dwt_last;
dwt_last{1} = original;
if layers>=1
    dwt_ca_1=dwt(original, dir_filter, 0);
    dwt_last = dwt_ca_1;                
    [h_1, w_1]=size(dwt_ca_1{1,1});     
end
if layers>=2
    dwt_ca_2=dwt(dwt_ca_1{1,1}, dir_filter, 0);
    dwt_last = dwt_ca_2;                
    [h_2, w_2]=size(dwt_ca_2{1,1});     
end
if layers>=3
    dwt_ca_3=dwt(dwt_ca_2{1,1}, dir_filter, 0);
    dwt_last = dwt_ca_3;                
    [h_3, w_3]=size(dwt_ca_3{1,1});     
end

[h_dwt, w_dwt]=size(dwt_last{1,1});       

%%%% Freq.zone divided into blocks %%%
% Subsequent band coeffs -> subsequent coeffs of subsequent band blocks
% Put into array dwt_gathered
% Store array indices of block elements in ind{i_block}

windowX_L_mat = (1:size_of_block:w_dwt);
windowY_L_mat = (1:size_of_block:h_dwt);
windowX_H_mat = min((1:size_of_block:w_dwt)+size_of_block-1,w_dwt);
windowY_H_mat = min((1:size_of_block:h_dwt)+size_of_block-1,h_dwt);

[windowX_L, windowY_L] = meshgrid(windowX_L_mat,windowY_L_mat);
[windowX_H, windowY_H] = meshgrid(windowX_H_mat,windowY_H_mat);

n_blocks = numel(windowX_L);
n_points = 0;
for i_block = 1:n_blocks
    windowX{i_block} = windowX_L(i_block):windowX_H(i_block);
    windowY{i_block} = windowY_L(i_block):windowY_H(i_block);
    n_p_this = numel(windowX{i_block})*numel(windowY{i_block});
    ind{i_block}=reshape(1:n_p_this,numel(windowY{i_block}),numel(windowX{i_block}))+n_points;
    n_points = n_points + n_p_this;
end

% Here the coeffs for operation are stored
dwt_gathered = repmat({zeros(n_points,1)},N,N);
for i_block=1:n_blocks
    for n=1:N
        for m=1:N
            dwt_gathered{m,n}(ind{i_block}) = dwt_last{m,n}(windowY{i_block},windowX{i_block});
        end
    end
    % Initialize the array of dequantized valuess
    dwt_gathered_deq = dwt_gathered;    
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

psnrs = zeros(qf_n,1);
entropys = zeros(qf_n,1);

qf_i = 0;
for qf = qf_list
    qf_i=qf_i+1;
    dwt_g = dwt_gathered{bandH,bandW};
    dwt_q = dwt_g;
    dwt_d = dwt_g;
    
    % Possible to quantize all but main considered band
    if ~skip_main
        shifts = zeros(n_blocks,1);

        if type == 8    % accelarated search
            N_half = (fast_rate-1)/2;
            k = -N_half:N_half;
            Fsq = 4*pi./(1:N_half).^2.*(-1).^(1:N_half);
            Fsq = [flip(Fsq), 2/3*pi^3, Fsq];

            dd = 1/qf;
            dw = 2*pi*qf;
            Fe_h = exp(-1j*dw*dwt_g.*(1:N_half));

            alpha_v = linspace(0,dd,shift_steps+1)/dd;
            alpha_v = alpha_v(1:end-1);
            exp_k_a = exp(1j*2*pi*k.'.*alpha_v);
        end

        for i_block = 1:n_blocks

            qf_this = qf; % qf for this block
            if shift_steps ~= 0
                d_shift = 1/qf_this/shift_steps;
            end
                
            % means to determine shift
            % by default it is 0 (type 1)
            switch (type)   % type of shift
                case 1      % no shift
                shift_this = 0;

                case 2      % Common mode
                shift_this = im_common_mode*delta^layers;

                case 3      % Pdf mode
                width = [];
                [f,xi,bw]=ksdensity(dwt_g(ind{i_block}(:)),0:d_shift:255*delta^layers,'width',width);
                [M,I]=max(f);
%                 shift_this = round(xi(I)/d_shift)*d_shift;
                shift_this = xi(I);

                case 4      % Rounded mean
                shift_this = round(mean(dwt_g(ind{i_block}(:)))/d_shift)*d_shift;

                case 5      % Rounded median
                shift_this = round(median(dwt_g(ind{i_block}(:)))/d_shift)*d_shift;

                case 6      % Adaptive
    %             d_shift = 1/shift_steps/qf_this;
                rms_best = +Inf;
                for shift_it = linspace(0, 1/qf_this*(1-1/shift_steps), shift_steps)
                    dwt_q(ind{i_block}) = round((dwt_g(ind{i_block})-shift_it)*qf_this);
                    dwt_d(ind{i_block}) = dwt_q(ind{i_block})/qf_this+shift_it;

                    rms_this = rms(dwt_d(ind{i_block}(:))-dwt_g(ind{i_block}(:)));
                    if rms_best > rms_this
                        rms_best = rms_this;
                        shift_best = shift_it;
                    end
                end
                shift_this = shift_best;

                case 7      % Fast adaptive
    %             d_shift = 1/shift_steps/qf_this;
    %             shift_steps = max(round(1/qf_this/d_shift),2);
                shift_this = get_best_shift(dwt_g(ind{i_block}), qf_this, fast_rate, shift_steps);

                case 8      % Faster adaptive
                Fg_h = sum(Fe_h(ind{i_block}(:),:),1);
                Fg = [conj(flip(Fg_h)), numel(dwt_g(ind{i_block})), Fg_h];
                er_v = (Fg.*Fsq)*exp_k_a;
                [M,I]=min(er_v);
                shift_this = dd*alpha_v(I);
            end
            
            % Quantizsation
            dwt_q(ind{i_block}) = quant_z(dwt_g(ind{i_block})-shift_this, qf_this, null_zone_extension);
%             dwt_q(ind{i_block}) = quant_r(dwt_g(ind{i_block})-shift_this, qf_this);
            if any(type == [6 7 8])   % Both adaptive modes, shift correction
                mr = round(mode(dwt_q(ind{i_block}(:))));
                dwt_q(ind{i_block}) = dwt_q(ind{i_block}) - mr;
                shift_this = shift_this + mr/qf_this;
            end
            
            % Dequantization
            dwt_d(ind{i_block}) = dequant_z(dwt_q(ind{i_block}), qf_this, null_zone_extension)+shift_this;
%             dwt_d(ind{i_block}) = dequant_r(dwt_q(ind{i_block}), qf_this)+shift_this;

            % Creating array of shifts
            shifts(i_block) = shift_this;

        end
        
        % Put restored into array
        dwt_gathered_deq{bandH, bandW} = dwt_d;
        % Update entropy (band + shifts)
        entropys(qf_i) = entropy(dwt_q)+entropy(shifts);
    else
        entropys(qf_i) = 0;
    end

    % Backwards block transform: from blocks to concatenated band
    dwt_last_q = repmat({zeros(h_dwt, w_dwt)},N,N);
    for i_block=1:n_blocks
        for n=1:N
            for m=1:N
                dwt_last_q{m,n}(windowY{i_block},windowX{i_block}) = dwt_gathered_deq{m,n}(ind{i_block});
            end
        end
    end
    
    % Initialization of other restored bands
    switch layers
        case 3
        dwt_ca_3r = dwt_last_q;
        dwt_ca_2r = dwt_ca_2;
        dwt_ca_1r = dwt_ca_1;
        
        case 2
        dwt_ca_2r = dwt_last_q;
        dwt_ca_1r = dwt_ca_1;
        
        case 1
        dwt_ca_1r = dwt_last_q;
        
        case 0
        restored = dwt_last_q{1};
    end
    
    % L3: Regular quantization, storing
    % qf_this = 0 skips quantization (no contribution to psnr and entropy)
    if numel(qf_extension_3)==1
        qf_this = qf*qf_extension_3;
    else
        qf_this = qf*qf_extension_3(qf_i);
    end
    if layers >= 3 && qf_this ~= 0
        for n_b=2:9
            if numel(null_zone_extension_3)==1
                null_zone_extension_this = null_zone_extension_3;
            else
                null_zone_extension_this = null_zone_extension_3(qf_i);
            end
            dwt_q3 = quant_z(dwt_ca_3r{n_b}, qf_this, null_zone_extension_this);
            dwt_ca_3r{n_b} = dequant_z(dwt_q3, qf_this, null_zone_extension_this);
            entropys(qf_i) = entropys(qf_i) + entropy(dwt_q3);
        end
    end    

    % L2: Regular quantization, storing
    if numel(qf_extension_2)==1
        qf_this = qf*qf_extension_2;
    else
        qf_this = qf*qf_extension_2(qf_i);
    end
    if layers >= 2 && qf_this ~= 0
        for n_b=2:9
            if numel(null_zone_extension_2)==1
                null_zone_extension_this = null_zone_extension_2;
            else
                null_zone_extension_this = null_zone_extension_2(qf_i);
            end
            dwt_q2 = quant_z(dwt_ca_2r{n_b}, qf_this, null_zone_extension_this);
            dwt_ca_2r{n_b} = dequant_z(dwt_q2, qf_this, null_zone_extension_this);
            entropys(qf_i) = entropys(qf_i) + entropy(dwt_q2);
        end
    end
    
    % L1: Regular quantization, storing
    if numel(qf_extension_1)==1
        qf_this = qf*qf_extension_1;
    else
        qf_this = qf*qf_extension_1(qf_i);
    end
    if layers >= 1 && qf_this ~= 0
        for n_b=2:9
            if numel(null_zone_extension_1)==1
                null_zone_extension_this = null_zone_extension_1;
            else
                null_zone_extension_this = null_zone_extension_1(qf_i);
            end
            dwt_q1 = quant_z(dwt_ca_1r{n_b}, qf_this, null_zone_extension_this);
            dwt_ca_1r{n_b} = dequant_z(dwt_q1, qf_this, null_zone_extension_this);
            entropys(qf_i) = entropys(qf_i) + entropy(dwt_q1);
        end
    end
     
    if layers>=3
        dwt_ca_2r{1,1} = idwt(dwt_ca_3r, inv_filter, 0, sym_coef, h_2, w_2);
    end
    
    if layers>=2
        dwt_ca_1r{1,1} = idwt(dwt_ca_2r, inv_filter, 0, sym_coef, h_1, w_1);
    end
    
    if layers>=1
        restored=idwt(dwt_ca_1r, inv_filter, 0, sym_coef, h_or, w_or);
    end

    psnrs(qf_i) = psnr(original,restored,255);

end
if ~relative
    plot(entropys,psnrs,line_type,'DisplayName',idea);
end

legend('Location','best');
axis tight
if tictoc 
    toc
end
