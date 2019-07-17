for Nlog_this=1088:1090
    Yid = importdata(['log/log_', num2str(Nlog_this), '/Y_log_short.txt']);
    bitmap=Yid.textdata(2:end,1);

    Nbands=9;

    for rb_out = string(unique(categorical(bitmap))).'
        figure(2);
        hold on;
        cla;    
        for nb_out = 1:9

            markerstyle='-';
            markerstyle_orig='-';
            add=false;
            addOriginal=true;
            Nlog=Nlog_this;
            figure(1);
            grapher
            dref = datax0_all(ind);
            add=false;
            Nlog=Nlog_this+12;
            figure(1);
            grapher
            figure(2)
            plot (datay_all(ind), datax0_all(ind)./dref*100,  markerstyle_orig, 'LineWidth', 1.0, 'DisplayName', bands(nb_out).name);
            xlabel('PSNR, dB');
            ylabel('Сжатие по энтропии, %');
            legend show
        end
        savefig(fname_fig);
        saveas(gcf,fname_bmp);
    end
end