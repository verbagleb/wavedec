% Nlog=318;   % number of log set
% Nbands=1;   % number of bands in log
mode=6;     % graph mode:
                %1: mode - entropy(L/S/H)
                %2: image - entropy(L/S/H)
                %3: quant_factor - entropy(each band)
                %4: quant_factor - entropy(S)
                %5: PSNR - entropy(each band)
                %6: PSNR - entropy(L) - for block
                %7: universal (argument - func for parameter)
                %8: PSNR - entropy - for division
sorting = true;
            
split_size_code=100;    %indicates usage of split with (block_size-split_size_code)
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
if sorting
    quant_factor = Yid.data(:,3);
    [quant_factor_sorted,sort_ind]=sort(quant_factor);
    Yid.data=Yid.data(sort_ind,:);
    Yid.textdata=Yid.textdata([1; sort_ind+1],:);
end
header=Yid.textdata(1,2:end);
bitmap=Yid.textdata(2:end,1);
block_size = Yid.data(:,1);
pred_mode = Yid.data(:,2);
quant_factor = Yid.data(:,3);
Y.Or_L = Yid.data(:,4);
Y.Pr_L = Yid.data(:,5);
Y.Or_S = Yid.data(:,4+2*Nbands);
Y.Pr_S = Yid.data(:,5+2*Nbands);
Y.Or_H = Y.Or_S - Y.Or_L;
Y.Pr_H = Y.Pr_S - Y.Pr_L;
Y.Or = Yid.data(:,4:2:2+2*Nbands);
Y.Pr = Yid.data(:,5:2:3+2*Nbands);
Y.PSNR = Yid.data(:,6+2*Nbands:5+3*Nbands);
Y.Extra = Yid.data(:,6+3*Nbands);
% if coef
%     Y.Extra2 = Yid.data(:,7+3*Nbands);
% end


% Crid = importdata(['log/log_', num2str(Nlog), '/Cr_log_short.txt']);
% Cr.Or_L = Crid.data(:,4);
% Cr.Pr_L = Crid.data(:,5);
% Cr.Or_S = Crid.data(:,4+2*Nbands);
% Cr.Pr_S = Crid.data(:,5+2*Nbands);
% Cr.Or_H = Cr.Or_S - Cr.Or_L;
% Cr.Pr_H = Cr.Pr_S - Cr.Pr_L;
% Cr.Or = Crid.data(:,4:2:2+2*Nbands);
% Cr.Pr = Crid.data(:,5:2:3+2*Nbands);
% Cr.PSNR = Crid.data(:,6+2*Nbands:5+3*Nbands);
% Cr.Extra = Crid.data(:,6+3*Nbands);
% % if coef
% %     Cr.Extra2 = Crid.data(:,7+3*Nbands);
% % end
% 
% Cbid = importdata(['log/log_', num2str(Nlog), '/Cb_log_short.txt']);
% Cb.Or_L = Cbid.data(:,4);
% Cb.Pr_L = Cbid.data(:,5);
% Cb.Or_S = Cbid.data(:,4+2*Nbands);
% Cb.Pr_S = Cbid.data(:,5+2*Nbands);
% Cb.Or_H = Cb.Or_S - Cb.Or_L;
% Cb.Pr_H = Cb.Pr_S - Cb.Pr_L;
% Cb.Or = Cbid.data(:,4:2:2+2*Nbands);
% Cb.Pr = Cbid.data(:,5:2:3+2*Nbands);
% Cb.PSNR = Cbid.data(:,6+2*Nbands:5+3*Nbands);
% Cb.Extra = Cbid.data(:,6+3*Nbands);
% % if coef
% %     Cb.Extra2 = Cbid.data(:,7+3*Nbands);
% % end

switch mode
    case 1         
        fmask = 'Graphs/Mode-compression/%s/mode_compression_%s.%s';
%         for restrict_bitmap = string(unique(categorical(bitmap))).'
        for restrict_bitmap = {'kiel.bmp'}
        
            subplot(1,2,1);
            restrict_block_size = 4;
            cond = categorical(bitmap)==restrict_bitmap & block_size == restrict_block_size;

            comp = Y; %Y / Cr / Cb
            Or = comp.Or(cond,5); %L / S / H
            Pr = comp.Pr(cond,5);
%             Or = comp.Or_L(cond,:); %L / S / H
%             Pr = comp.Pr_L(cond,:);
            suffix = ' - LL';
            argument = categorical(pred_mode(cond,:));

            data_y = Pr./Or*100;
            bar(argument, data_y, 0.5);
            title([ num2str(Nlog), ': Compression after prediction over original entropy on mode ', char(restrict_bitmap), suffix]);
            xlabel('mode');
            ylabel('Ent-Pred/Ent-Orig, %');
            ylim([95 100]);
            
            subplot(1,2,2);
            restrict_block_size = 4;
            cond = categorical(bitmap)==restrict_bitmap & block_size == restrict_block_size;

            comp = Y; %Y / Cr / Cb
            Or = comp.Or(cond,2); %L / S / H
            Pr = comp.Pr(cond,2);
%             Or = comp.Or_H(cond,:); %L / S / H
%             Pr = comp.Pr_H(cond,:);
            suffix = ' - HH';
            argument = categorical(pred_mode(cond,:));

            data_y = Pr./Or*100;
            bar(argument, data_y, 0.5);
            title([ num2str(Nlog), ': Compression after prediction over original entropy on mode ', char(restrict_bitmap), suffix]);
            xlabel('mode');
            ylabel('Ent-Pred/Ent-Orig, %');
            ylim([95 100]);
            return;
            
            fname = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4), 'fig');
            savefig(fname);
            fname = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), 'bmp');
            saveas(gcf,fname);
        end
        
    case 2
        comp = Y; %Y / Cr / Cb
%         Or = comp.Or_S(1:end/2,:); %L / S / H
%         Pr = comp.Pr_S(1:end/2,:);
%         bs = block_size(1:end/2,:);
        Or = comp.Or(1:end,2); %L / S / H
        Pr = comp.Pr(1:end,2);
        bs = block_size(1:end,:);
        one_bar = false;
        
        parameter_values=unique(bs);
        data_y=[];
        leg=cell(1,numel(parameter_values)+1);
        for j=1:numel(parameter_values)
            pv=parameter_values(j);
            ind = bs==pv;
            data_y = [data_y, Pr(ind)];
            if pv<0
                leg{j} = [num2str(-pv-100), '-split'];
            else
                leg{j} = [num2str(pv), 'x', num2str(pv)];
            end
        end
        leg{end}='Orig';
        data_y = [data_y, Or(ind)];
        parameter='';
        if one_bar
            data_y = [data_y(:,1) diff(data_y,1,2)];
            parameter='stacked';
        end
        arg = categorical(bitmap(ind));
        b=bar(arg, data_y, 0.5, parameter);
        for pln = 1:numel(parameter_values)
            b(pln).FaceAlpha=0.5;
        end
        legend(leg);
        title([ num2str(Nlog), ': Entropy on image']);
        xlabel('image');
        ylabel('Entropy, bits');
        
    case 3
        comp = Y; %Y / Cr / Cb
        data_y = comp.Pr./comp.Or;
        plot(1./quant_factor, data_y);
        legend(header(:,5:2:3+2*Nbands));
        title([ num2str(Nlog), ': Entropy after prediction by band']);
        xlabel('1/quant\_factor');
        ylabel('Ent-Pred_j, bits')
        
    case 4
        comp = Y; %Y / Cr / Cb
        data_y = comp.Pr_L./comp.Or_L;
        plot(1./quant_factor, data_y);
        title([ num2str(Nlog), ': Common entropy after prediction to common original entropy']);
        xlabel('1/quant\_factor');
        ylabel('Ent-Pred_S/Ent-Orig_S');
        
    case 5
%         add = true;
        addOriginal = false;
        crop = true;
%         fmask = 'Graphs/Quant-test/2/%s/bitrate-PSNR_%s.%s';
%         fmask = 'Graphs/Filters/3/Conditional/DeadZone-2/%s/bitrate-PSNR_%s.%s';
%         fmask = 'Graphs/AbsSgn/Separate/%s/abssgn_bP_%s_%dx%d.%s';
        comp = Y; %Y / Cr / Cb
        
%         for restrict_bitmap = string(unique(categorical(bitmap))).'
        for restrict_bitmap = {'kiel.bmp'}
            if add
                close;
                fname = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4), 'fig');
                openfig(fname);
                hold on;
            else
                clf;
            end
%             for restrict_block_size = restrict_block_size_outer
            for restrict_block_size = 2
                if add
                    Legend = get(gca,'legend');
                    leg_old = Legend.String;
                    hold on;
                end
                cond = block_size == restrict_block_size & categorical(bitmap)==restrict_bitmap;
                [X,I]=sort(comp.Or_S(cond));
                Yuns = comp.Extra(cond);
                plot(X, Yuns(I),'-o','LineWidth', lw_outer);
                if addOriginal
                    hold on;
                    plot(comp.Or(cond,2:9), comp.PSNR(cond,2:9),'--o');
                    leg = [header(:,7:2:3+2*Nbands), header(:,6:2:2+2*Nbands)];
                else
                    leg = leg_outer;
%                     leg = num2str(restrict_block_size);
                end
                hold off;
                title([ num2str(Nlog), ': PSNR-bitrate by band (', restrict_bitmap{1}, ')']);
                xlabel('Entropy, bits');
                ylabel('PSNR, dB');
                if add
                    leg = [leg_old leg];    
                end
                legend(leg);
            end
%             title(legend, 'Dead zone');
            legend(leg, 'Location', 'southeast');
            axis tight
            fname = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4),  'fig');
            savefig(fname);
            fname = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4),  'bmp');
            saveas(gcf,fname);
            if crop
                ylim([34 42])
%                     fname = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), '_crop', 'bmp');
                fname = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4),  'crop.bmp');
                saveas(gcf,fname);
            end
            
        end 
        
    case 6
%         add = true;
%         fmask = 'Graphs/Context/Original-2/%s/abssgn_bP_%s_H%s.%s';
%         fmask = 'Graphs/Strange/2/%s/abssgn_bP_%s_b%d%s_compare_lbr.%s';
        fmask = 'Graphs/Intra/AbsSign-rel/%s/abssgn_bP_%s_b_all%s.%s';
%         markerstyle= '--o';
%         markerstyle_orig = ':s';
        comp = Y; %Y / Cr / Cb
        swap_xy = false;
        suffix = '';
        chooseBest = true;
        crop = false;
%         addOriginal = false;
%         no_leg = true;

%         for restrict_bitmap = string(unique(categorical(bitmap))).'
        for restrict_bitmap = rb_out
%             for nband = 1:Nbands
            for nband = nb_out
                if add
                    close;
%                     fname = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4), '', 'fig');
                    fname = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4),  '', 'fig');
                    openfig(fname);
                    hold on;
                    Legend = get(gca,'legend');
                    leg_old = Legend.String;
                    set(gca,'ColorOrderIndex',1);
                else
                    clf;
                    co7=get(gca,'ColorOrder');
                    assert(size(co7,1)==7);
                    co11=[co7; 0.9 0.5 0.1;  1.0000    0.5882    0.66676; 0.5 0.5 0.5; 0 0 0];
                    plot([], []);
                    set(gca,'ColorOrder',co11);
                    hold on;
                end
                extra_cond = any(block_size==4,2);
                cond = categorical(bitmap)==restrict_bitmap & extra_cond;

                entP_full = comp.Pr_S(cond,:);
                entO_full = comp.Or_S(cond,:);
                entP = comp.Pr(cond,nband);
                entO = comp.Or(cond,nband);

                datax_all = entP;
                datax0_all = entO;
%                 datax_all = entP(end/2+1:end,:)./entP(1:end/2,:)*100.0;
%                 datax0_all = entO(end/2+1:end,:)./entO(1:end/2,:)*100.0;
%                 datax_all = entP(end/2+1:end,:);
%                 datax0_all = entO(end/2+1:end,:);
%                 datax_all = entP(1:end/2,:);
%                 datax0_all = entO(1:end/2,:);

%                 psnr = comp.Extra;
                psnr = comp.PSNR(cond,nband);
                psnr_full = comp.Extra(cond);
                
                datay_all = psnr_full;
%                 datay_all = psnr(1:end/2,:);
                
                qf = quant_factor(cond,:);
                pm = pred_mode(cond,:);
                bs = block_size(cond,:);
%                 bs = bs(1:end/2,:);
                leg_orig = 'Original';
%                 leg_orig = 'Abs-sgn (with context)';

                parameter_values=unique(pm);
                parameter2_values=unique(bs);
                datax=[];
                datay=[];
                leg = {};
%                 leg=cell(1,numel(parameter_values));
                for j=1:numel(parameter_values)
                    for j2=1:numel(parameter2_values)
                        pv=parameter_values(j);
                        p2v=parameter2_values(j2);
                        ind = (pm==pv & bs==p2v);
                        datax(:,j,j2) = datax_all(ind);
                        datay(:,j,j2) = datay_all(ind);
                        if pv>split_size_code
                            leg{j} = [num2str(pv-100), '-split'];
                        else
    %                         leg{j} = [num2str(pv), 'x', num2str(pv)];
                            leg{j} = ['mode ', num2str(pv)];
                        end
                    end
                end
                
                psnr_full_w=psnr_full(psnr_full>34 & psnr_full<42 & ind,:);
                psnr_w=psnr(psnr_full>34 & psnr_full<42 & ind,:);
%                 lh_p=interp1(psnr_full_w, psnr_w, [34; 42], [],'extrap');

                if chooseBest
                    [datax, ind_best] = min(datax, [], 3);
                    datay = datay(:,:,1);
                    for j=1:size(leg,2)
                        if ~any(diff(ind_best(psnr_full(ind)>30 & psnr_full(ind)<46,j),[],1),1)
                            leg{j} = [leg{j}, ' - ', num2str(parameter2_values(ind_best(1,j))), ' (best)'];
                        else
                            leg{j} = [leg{j} ' - Best pred'];
                        end
                    end
                end
%                 
                if swap_xy
                    plot(datay, datax, markerstyle);
                    ylabel('Entropy, %');       
                    xlabel('PSNR, dB');
                    if addOriginal
                        hold on;
                        plot (datay_all(ind), datax0_all(ind), markerstyle_orig, 'LineWidth', 1.0);
                        leg = [leg, leg_orig];
                    end
                else
%                     plot(datax, datay, markerstyle);
                    xlabel('Entropy, bits');       
                    ylabel('PSNR, dB');
                    if addOriginal
                        hold on;
                        plot (datax0_all(ind), datay_all(ind), markerstyle_orig, 'LineWidth', 1.0);
                        leg = [leg, leg_orig];
                    end
                end
                hold off;
%                 title([ num2str(Nlog), ': PSNR-bitrate for block size (', restrict_bitmap{1}, ')', suffix]);
                title([string([ num2str(Nlog), ': PSNR-bitrate for prediction mode']); string([restrict_bitmap{1}, ' - band ', num2str(nband), suffix])]);
                if add
                    leg = [leg_old leg];    
                end
                legend(leg, 'Location', 'best');
%                 if no_leg
%                     leg = leg_old;  % meanwhile the names are kept
%                     legend(leg, 'Location', 'best');
%                 end
%                 title(legend,'Mode');
%                 title(legend,["No skipping (solid)"; "Skipping allowed (dash-dot)"; "Skipping allowed"; "(flags omitted, dashed)"]);
                title(legend,["Unshifted"; "Centered"]);
                axis tight;
                fname_bmp = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), '', 'bmp');
%                 fname_bmp = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), nband, '', 'bmp');
                saveas(gcf,fname_bmp);
                if crop
%                     ylim([34 42])
                    ylim([min(lh_p) max(lh_p)]);
                    fname_crop = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), '_crop', 'bmp');
%                     fname_crop = sprintf(fmask, 'bmp', restrict_bitmap{1}(1:end-4), nband, '_crop', 'bmp');
                    saveas(gcf,fname_crop);
                end
                fname_fig = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4), '', 'fig');
%                 fname_fig = sprintf(fmask, 'fig', restrict_bitmap{1}(1:end-4), nband, '', 'fig');
                savefig(fname_fig);
            end
        end
        
    case 8
%         add = true;
%         subband = 'HL';
%         markerstyle = 'o-';
        fmask = 'Graphs/Division/%s/5t-high/%s/%s-div_%s_%s%s.%s';
                
        comp = Y; %Y / Cr / Cb
        comp.Or_LS=sum(comp.Or(:,1:Nbands), nsb);
        comp.Pr_LS=sum(comp.Pr(:,1:Nbands), nsb);
        
        if half==true
            datax_full = comp.Pr_L + comp.Or_H;
        else
            datax_full = comp.Pr_S;
        end
        dataxO_full = comp.Or_S;
        datay_full = comp.Extra;

        swapxy = false;
%         addOriginal = true;
        chooseBest = true;
        legloc = 'northwest';
        xlabeltext = 'Entropy, bits';
        datamode = 'Or_Pr';
        extra_cond = true;% datay_full > 40 & datay_full < 60;

%         data_S = comp.Or_LS-comp.Pr_LS;
%         swapxy = true;
%         addOriginal = false;
%         legloc = 'southwest';
%         xlabeltext = 'Ent_{orig}-Ent_{pred}, bits';
%         datamode = 'abs';
        
%         data_S = comp.Pr_LS./comp.Or_LS*100;
%         swapxy = true;
%         addOriginal = false;
%         legloc = 'southeast';
%         xlabeltext = 'Ent_{pred}/Ent_{orig}, %';        
%         datamode = 'rel';
         
        for restrict_bitmap = string(unique(categorical(bitmap))).'
            if add
                close;
                fname = sprintf(fmask, subband, 'fig', subband, restrict_bitmap{1}(1:end-4), datamode, '', 'fig');
                openfig(fname);
                hold on;
                Legend = get(gca,'legend');
                leg_old = Legend.String;
            end
            
            cond = categorical(bitmap)==restrict_bitmap & extra_cond;
            entP = datax_full(cond,:);
            entO = dataxO_full(cond,:);
            if coef
                entP = entP + comp.Extra2(cond,:);
                entO = entO + comp.Extra2(cond,:);
            end
            bs = block_size(cond,:);
            suffix = '';

            parameter_values=unique(bs);
            datax=[];
            datay=[];
            leg=cell(1,numel(parameter_values));
            for j=1:numel(parameter_values)
                pv=parameter_values(j);
                ind = (bs==pv);
                datax = [datax, entP(ind)];
                datay = [datay, datay_full(ind)];    
                if pv>split_size_code
                    leg{j} = [num2str(pv-100), '-split'];
                else
                    leg{j} = [num2str(pv), 'x', num2str(pv)];
                end
            end
            if chooseBest
                [datax, ind_best] = min(datax, [], 2);
                datay = datay(:,1);
                if ~any(diff(ind_best))
                    leg = {[leg{ind_best(1)}, ' (best)']};
                else
                    leg = {'Best pred'};
                end
            end

            if swapxy
                plot(datay, datax, markerstyle);
            else
                plot(datax, datay, markerstyle);
            end
            if addOriginal
                hold on;
                if swapxy
                    plot(datay_full(ind), entO(ind), markerstyle, 'LineWidth', 1.5);
                else
                    plot(entO(ind), datay_full(ind), markerstyle, 'LineWidth', 1.5);
                end
                leg{end+1} = 'Original';
            end
            hold off;
            if add
                leg = [leg_old leg];
            end
            title([ num2str(Nlog), ': PSNR-bitrate for ' subband, '-band block size (', restrict_bitmap{1}, ')', suffix]);

            ylabeltext = 'PSNR, dB';
            if swapxy
                ylabel(xlabeltext);       
                xlabel(ylabeltext);
            else
                xlabel(xlabeltext);       
                ylabel(ylabeltext);
            end
            legend(leg, 'Location', legloc);
            title(legend,["Division (full-Pr, half-Pr, Or)";"Abs->Division(3.0, 5.0, 7.0)";"Simple (Pr, Or)"])
            if addText
%                 gtext(text);
            end
            axis tight;
            fname = sprintf(fmask, subband, 'fig', subband, restrict_bitmap{1}(1:end-4), datamode, '', 'fig');
            savefig(fname);
            fname = sprintf(fmask, subband, 'bmp', subband, restrict_bitmap{1}(1:end-4), datamode, '', 'bmp');
            saveas(gcf,fname);
            ylim([35 45]);
%             ylim([50 50.3]);
%             ylim([34 42]);
            fname = sprintf(fmask, subband, 'bmp', subband, restrict_bitmap{1}(1:end-4), datamode, '_crop', 'bmp');
            saveas(gcf,fname);
        end
        
    case 7

        comp = Y; %Y / Cr / Cb
        Or = comp.Or_L;
        Pr = comp.Pr_L;
        suffix = ' - L';
        func = (-Pr(1:end/2,:)+Pr(end/2+1:end,:))./Pr(end/2+1:end,:)*100;    % profit
        func_name = 'Relative entropy change, %';
%         func_orig = Or(1:end/2,:);
        parameter = pred_mode(1:end/2,:);
        parameter_name = 'Prediction mode';
        argument = block_size(1:end/2,:); 
        argument_name = 'Block size';
        var1 = string(categorical(bitmap(1:end/2,:)));
        var1_name = 'image';
        var2 = quant_factor(1:end/2,:);
        var2_name = 'quantization factor';
        extra_cond = true;
%         extra_cond = parameter==10;

        bar_type = false;
        one_bar = false;
        addOriginal = false;
        
        fmask = 'Graphs/Filtering/%s/filtering_all_L_%s_%f.%s';

%         func = comp.Extra;
%         parameter = block_size;
%         argument = Or-Pr;
% %         argument = Pr./Or*100;

        for var1_i = unique(var1).'
            for var2_i = unique(var2).'
                cond = var1==var1_i & var2==var2_i & extra_cond; 
                parameter_i = parameter(cond);
                argument_i = argument(cond);
                func_i = func(cond);
                
                parameter_values=unique(parameter_i);
                data_x=[];
                data_y=[];
                leg=cell(1,numel(parameter_values));
                for j=1:numel(parameter_values)
                    pv=parameter_values(j);
                    ind = parameter_i==pv;
                    data_x = [data_x, argument_i(ind)];
                    data_y = [data_y, func_i(ind)];
                    leg{j}=num2str(pv);% [ num2str(pv) 'x' num2str(pv)];
                end

                if addOriginal
                    data_x = [data_x, argument_i(ind)];
                    data_y = [data_y, func_orig(ind)];
                    leg = [leg 'Original'];
                end

                if bar_type
                    if one_bar
                        data_y = [data_y(:,1) diff(data_y,1,2)];
                        bar_parameter='stacked';
                    else
                        bar_parameter='';
                    end
                    bar(categorical(data_x), data_y, 0.5, bar_parameter);   % argument should have the same values for different values of parameter
                else
                    plot_parameter='-';
                    plot(data_x, data_y, plot_parameter);
                    axis tight;
                end
                title([ string([num2str(Nlog), ': Profit of filtering for each mode',  suffix])
                        string([var1_name, ' = ', var1_i{1}, ', ',  var2_name, ' = ', num2str(var2_i)])  ]);
                xlabel(argument_name);
                ylabel(func_name);
                legend(leg, 'Location', 'southeast');
                title(legend, parameter_name);
                
                fname = sprintf(fmask, 'fig', var1_i{1}(1:end-4), var2_i, 'fig');
                savefig(fname);
                fname = sprintf(fmask, 'bmp', var1_i{1}(1:end-4), var2_i, 'bmp');
                saveas(gcf,fname);

            end
        end
end