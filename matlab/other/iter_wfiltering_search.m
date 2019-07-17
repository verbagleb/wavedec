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

tictoc = true;
load_plots = false;
save_plots = false;
save_cropped = false;
relative = true;

folder_mask = 'Graphs/Quant/Separate profit/';
size_of_block = 5;
null_zone_extension = 1.0;  % defaultd

fig_num=0;
for band = bands(1)
    for image = images(1)
        folder = [folder_mask];
        im_name = ['files/no filter/' image.name '/fileSub_Y_0_orig.txt'];
        im_common_mode = image.common_mode;
%         figname = [folder image.name '_' num2str(size_of_block) 'x' num2str(size_of_block) '.fig'];
%         bmpname = [folder image.name '_' num2str(size_of_block) 'x' num2str(size_of_block) '.bmp'];
        figname = [folder image.name '.fig'];
        bmpname = [folder image.name '.bmp'];
        cropname = [folder image.name '_crop.bmp'];
        titlelabel = [image.name ' - Complex, profit'];
        if load_plots
            openfig(figname);
        else
            fig_num = fig_num+1;
            figure(fig_num);
            cla;
        end
        hold on;
        set(gcf,'Position', [400 200 1025 650]);
        bandW = band.W;
        bandH = band.H;

        line_type = '-';
        layers = 3;
        qf_n = 50;
        qf_list = 10.^(linspace(log10(0.3), log10(0.01),qf_n));
        shift_steps = Inf;
        
             
        type = 1;
        idea = '0';
        skip_main = false;
        null_zone_extension_1 = 1.0;
        null_zone_extension_2 = 1.0;
        null_zone_extension_3 = 1.0;
        qf_extension_1 = 1.0;
        qf_extension_2 = 1.0;
        qf_extension_3 = 1.0;
        wfiltering;
        entropys_0 = entropys;
        psnrs_0 = psnrs;

            
%         i_plot = i_plot + 1;
%         type = 5;
%         size_of_block = 5;
%         idea = '3';
%         null_zone_extension_1 = 1.0;
%         null_zone_extension_2 = 1.0;
%         null_zone_extension_3 = 1.0;
%         qf_extension_1 = 1.0;
%         qf_extension_2 = 1.0;
%         qf_extension_3 = 1.0;
%         shift_steps = 3;
% %             fast_rate = 5;
%         wfiltering;
%         entropys_i(:,i_plot) = entropys;
%         psnrs_i(:,i_plot) = psnrs;

        null_zone_extension_1_ar = linspace(1,2.3,100);
        null_zone_extension_2_ar = linspace(1,2.3,100);
        qf_extension_1_ar = linspace(1,1.5,100);
        qf_extension_2_ar = linspace(1,1.5,100);
        qf_extension_3_ar = linspace(1,1.5,100);

        i_plot = 0;
        for null_zone_extension_1 = null_zone_extension_1_ar
            for qf_extension_1 = qf_extension_1_ar

                i_plot = i_plot + 1;
                type = 5;
                size_of_block = 5;
                idea = num2str(i_plot);
                skip_main = true;
        %         null_zone_extension_1 = 1.5;
                null_zone_extension_2 = 1.5;
                null_zone_extension_3 = 1.0;
        %         qf_extension_1 = 1.0;
                qf_extension_2 = 0;
                qf_extension_3 = 0;
                shift_steps = 3;
        %             fast_rate = 5;
                wfiltering;
                entropys_12(:,i_plot) = entropys;
                psnrs_12(:,i_plot) = psnrs;

            end
        end

        i_plot = 0;
        for null_zone_extension_2 = null_zone_extension_2_ar
            for qf_extension_2 = qf_extension_2_ar

                i_plot = i_plot + 1;
                type = 5;
                size_of_block = 5;
                idea = num2str(i_plot);
                skip_main = true;
                null_zone_extension_1 = 1.5;
        %         null_zone_extension_2 = 1.5;
                null_zone_extension_3 = 1.0;
                qf_extension_1 = 0;
        %         qf_extension_2 = 1.0;
                qf_extension_3 = 0;
                shift_steps = 3;
        %             fast_rate = 5;
                wfiltering;
                entropys_34(:,i_plot) = entropys;
                psnrs_34(:,i_plot) = psnrs;

            end
        end

        i_plot = 0;
        for qf_extension_3 = qf_extension_3_ar

            i_plot = i_plot + 1;
            type = 5;
            size_of_block = 5;
            idea = num2str(i_plot);
            skip_main = true;
            null_zone_extension_1 = 1.5;
            null_zone_extension_2 = 1.5;
            null_zone_extension_3 = 1.0;
            qf_extension_1 = 0;
            qf_extension_2 = 0;
        %         qf_extension_3 = Inf;
            shift_steps = 3;
        %             fast_rate = 5;
            wfiltering;
            entropys_5(:,i_plot) = entropys;
            psnrs_5(:,i_plot) = psnrs;

        end

        type = 5;
        size_of_block = 5;
        idea = num2str(i_plot);
        skip_main = false;
        null_zone_extension_1 = 1.5;
        null_zone_extension_2 = 1.5;
        null_zone_extension_3 = 1.0;
        qf_extension_1 = 0;
        qf_extension_2 = 0;
        qf_extension_3 = 0;
        shift_steps = 3;
        %             fast_rate = 5;
        wfiltering;
        entropys_m = entropys;
        psnrs_m = psnrs;

        % save entropys_i
        % save psnrs_i

        save(['files/Look_through/entropys&psnrs_' image.name '_big'], 'entropys_0', 'entropys_12', 'entropys_34', 'entropys_5', 'entropys_m',...
                'psnrs_0', 'psnrs_12', 'psnrs_34', 'psnrs_5', 'psnrs_m');
    end
end
return
        if relative  
            
            n_p=150;
            
    %         e_m = min([entropys_0;entropys_i(:)]);
    %         e_M = max([entropys_0;entropys_i(:)]);
            p_m = min([psnrs_0;psnrs_i(:)]);
            p_M = max([psnrs_0;psnrs_i(:)]);
%             p_v = linspace(p_m,p_M,n_p).';
            p_v = 45;
            e_0 = interp1(psnrs_0,entropys_0,p_v,'pchip');
            for i_plot = 1:size(psnrs_i,2)
                e(:,i_plot) = interp1(psnrs_i(:,i_plot),entropys_i(:,i_plot),p_v,'pchip');
            end

            figure
            hold on;
            set(gcf,'Position', [400 200 1025 650]);
%             plot(p_v,e./e_0*100);
%             if ~load_plots
%                 plot(p_v,100*ones(1,n_p),'k--');
%             end
            e2 = reshape(e./e_0*100, numel(iter1_ar), numel(iter2_ar));
            [I1, I2] = meshgrid(iter1_ar, iter2_ar);
            mesh(I1,I2,e2);
            xlabel('iter1')
            ylabel('iter2')
%             legend({'[1] Median(10) shift of 3L, 5x5', '[2] 1.5 null-zone of 2H', 'Both'});
%             legend({'[1] Median(10) shift of 3L, 5x5', '[2] 1.5 null-zone of 2H', '[3] 1.5 null-zone of 3H', '[1]+[2]', '[1]+[3]'});
%             legend({'Сдвиг НЧ 3 уровня на медиану в блоках 5x5', 
%                 'x1.5 нуль-зона ВЧ 1 уровня', 
%                 'x1.5 нуль-зона ВЧ 2 уровня',
%                 'Совокупное сжатие'});
            legend(num2str((1:size(psnrs_i,2)).'))
%             xlabel('PSNR, dB');
%             ylabel('Relative entropy, %');
            title(titlelabel);
            
        else        % not relative
% 
%             null_zone_extension_1 = 1.0;
%             null_zone_extension_2 = 1.0;
%             null_zone_extension_3 = 1.0;
%             qf_n = 50;
%             shift_steps = 1;
% 
%             line_type = '--';
%             idea = 'Равномерное квантование';
%             type = 1;
%             wfiltering;
% 
%             null_zone_extension_1 = 1.5;
%             null_zone_extension_2 = 1.5;
%             null_zone_extension_3 = 1.0;
%             type = 6;
%             shift_steps = 5;
%             line_type = '-';
%             idea = 'Предлагаемый метод';
%             type = 1;
%             wfiltering;

    %         %common
    %         qf_n = 50;
    %         line_type = '-';
    %         size_of_block_mem = size_of_block;
    %         size_of_block = Inf;

    %         type = 1;
    %         for null_zone_extension = [0.75 1.0 1.5 2.0 3.0]
    %             if null_zone_extension==1.0
    %                 idea = ['x1.0 (Normal)'];
    %             else
    %                 idea = ['x' num2str(null_zone_extension)];
    %             end
    %             wfiltering;
    %         end
    %         title(legend,'Расширение нуль-зоны');

    %         type = 1;
    %         idea = 'Без сдвига';
    %         wfiltering;
    % 
    %         type = 2;
    %         idea = 'Общая мода';
    %         wfiltering;

    %             type = 1;
    %             idea = 'No shift H1.5';
    %             null_zone_extension = 1.5;
    %             wfiltering;

    %             type = 1;
    %             idea = 'No shift 2.0';
    %             null_zone_extension = 2.0;
    %             wfiltering;

        %individual
    %         line_type = '-';
    %         size_of_block = size_of_block_mem;
    % 
    %         qf_n = 50;
    %         type = 3;
    %         idea = 'Мода';
    %         shift_steps = 3;
    %         wfiltering;
    % 
    %             idea = 'Rounded(10) mode';
    %             d_shift = 10;
    %             wfiltering;
    % % 
    %         qf_n = 50;
    %         type = 4;
    %         idea = 'Среднее';
    %         shift_steps = 3;
    %         wfiltering;
    % 
    %             idea = 'Rounded(10) mean';
    %             d_shift = 10;
    %             wfiltering;

    %         qf_n = 50;
    %         type = 5;
    %         idea = 'Медиана';
    %         shift_steps = 3;
    %         wfiltering;

    %          type = 6;
    %             idea = 'Adaptive(dd/5 shift)';
    %             line_type = '-';
    %             shift_steps = 5;
    %             wfiltering;
    % % % 
    %         idea = 'Перебор';
    %         line_type = '.-';
    %         shift_steps = 5;
    %         wfiltering;
    %        
    %             
    %             idea = 'Rounded(10) median H1.5';
    %             null_zone_extension = 1.5;
    %             d_shift = 10;
    %             wfiltering;

    %         type = 8;
    %         idea = 'Ускоренный перебор';
    %         line_type = '-o';
    %         shift_steps = 5;
    %         fast_rate = 5;
    %         wfiltering;

    % 
    %             idea = 'Fast adaptive(3-dd/20) shift';
    %             line_type = '-^';
    %             shift_steps = 20;
    %             fast_rate = 3;
    %             wfiltering;
    % 
    %             idea = 'Fast adaptive(3-dd/10) shift';
    %             shift_steps = 10;
    %             fast_rate = 3;
    %             wfiltering;

    %     layers = 3;
    %     line_type = '--';
    % 
    %         type = 1;
    %         idea = 'Без сдвига';
    %         wfiltering;

    %         for size_of_block = [4 3]
    %             qf_n = 50;
    %             type = 5;
    %             idea = 'Медиана';
    %             line_type = '-';
    %             shift_steps = 3;
    %             wfiltering;
    % 
    %             type = 6;
    %             idea = 'Перебор';
    %             line_type = '.-';
    %             shift_steps = 5;
    % %             fast_rate = 5;
    %             wfiltering;
    %         end

            xlabel('Энтропия, биты');
            ylabel('PSNR, dB');
            title(titlelabel);
        end
        shg;

        if save_plots
            savefig(figname);
            saveas(gcf, bmpname);
            if save_cropped
                yl=ylim;
                ylc = mean(yl);
                yld = diff(yl);
                c=1/3;
                ylim(ylc+[-1 1]*yld*c/2);
                saveas(gcf, cropname);
            end
        end
            
        
%     end
% end