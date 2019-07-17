Nbands = 9;
immask = 'Graphs/Distributions/pdf/2 level/%s/hist_%s_%s_b%d.%s';
% immask = 'Graphs/Distributions/error/%s/error_norm_%s_%s_b%d.%s';
% for bitmap_name = {'bitmap', 'bitmap\_4', 'bitmap\_5', 'kiel', 'goldhill', 'parkrun', 'stockholm'}
delta = 368.332995/128; %the response of LL to shift
for bitmap_name = {'kiel'}
    suffix = '';
    width=delta; % [] for auto
    %
    % windowX = 350:400;
    % windowY = 50:100;
    %
    windowX = 150:200;
    windowY = 250:300;
    bitmap_name_full= [erase(bitmap_name{1},'\') suffix];
    for comp = {'Y'} %'Y'/'Cb'/'Cr'
        fname = ['files/no filter/', bitmap_name_full, '/fileSub_', comp{1}, '_', num2str(1-1), '_orig.txt'];
        or = importdata(fname);
        dwt1=dwt(or, dir_filter, 0);   
%         dwt2=dwt(dwt1{1},dir_filter,0);
        for n=1
%            fname = ['files/no filter/', bitmap_name_full, '/fileSub_', comp{1}, '_', num2str(n-1), '_orig.txt'];

            component = or;
            component_crop = nan(size(component));
            component_crop(windowY,windowX)=component(windowY,windowX);
            component_col = component_crop(:);
    %         20*log10(256/rms(component_col))
            figure(n);
            hold off;
    %         stem(component_col,ones(size(component_col)));
    %         hold on
            set(gcf,'Position', [500 250 925 600]);
    %         qf_list = [2.0 1.0 0.5 0.25 0.1 0.05 0.025 0.01];
    %         for qf = qf_list
    %             error_col = component_col*qf - round(component_col*qf);
    %             ksdensity(error_col/qf, 'width', width);
    % %             ksdensity(error_col, 'width', width);
    %             hold on;
    %         end 
            [f,xi]=ksdensity(component_col, 'width', width);
            plot(xi,f);
%             histogram(component_col,256);
    %         legend(string(qf_list));
            legend('pdf');
            title(['Value density - ', bitmap_name{1}, ' - band ', num2str(n)]);
            xlabel('Y');
            ylabel('Part');
            axis tight
            if n~=1
                xlim(min(abs(xlim))*[-1, 1]);
            else
    %             xlim([0 255]);
            end    
    %         xlim([-0.5 0.5]*10);
    %         xlim([-0.5 0.5]);
%             xlim([0 255])
            ylim('auto');
            yl=ylim;
            ylim([0 yl(2)]);

            component_corr = component_col;
            [M,I]=max(f);
            mdpdf=xi(I);
            mdspl=zeros(1,5);
            for i=1:5
                mdspl(i)=mode(component_corr);
                component_corr(component_corr==mdspl(i))=NaN;
            end
            fprintf('%s\nPDF mode: %f\n5 sample modes: %f %f %f %f %f\nmedian: %f\nmean: %f\n\n', bitmap_name_full, mdpdf, mdspl, median(component_col, 'omitnan'), mean(component_col, 'omitnan'));


%             imname = sprintf(immask, 'fig', bitmap_name_full, comp{1}, n, 'fig');
%             savefig(imname);
%             imname = sprintf(immask, 'bmp', bitmap_name_full, comp{1}, n, 'bmp');
%             saveas(gcf, imname);
        end
    end
end