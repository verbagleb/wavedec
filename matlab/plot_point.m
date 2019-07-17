clear all

std_to_psnr = @(s)20*log10(255./s);
psnr_to_std = @(p)(255*10.^(-p/20));

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

qf_n = 50;
n_it_qf = 100;
    
directory = 'files/Look_through/';

for im_ind = []
    image = images(im_ind);

%     switch im_ind
%         case {1,2}
%             upper = 2.0;
%         case {3,4,5,6,7}
            upper = 2.3;
%     end
%     switch im_ind
%         case 5
%             n_it_nz = 26;
%             null_zone_extension_1_ar = [linspace(1,2,20) 2.05:0.05:2.30];
%             null_zone_extension_2_ar = [linspace(1,2,20) 2.05:0.05:2.30];
%         otherwise
            n_it_nz = 100;
            null_zone_extension_1_ar = linspace(1,upper,n_it_nz);
            null_zone_extension_2_ar = linspace(1,upper,n_it_nz);
%     end
    dimensions = [n_it_qf, n_it_nz, n_it_qf, n_it_nz, n_it_qf];
    
    qf_extension_1_ar = linspace(1,1.5,n_it_qf);
    qf_extension_2_ar = linspace(1,1.5,n_it_qf);
    qf_extension_3_ar = linspace(1,1.5,n_it_qf);
    qf_list = 10.^(linspace(log10(0.3), log10(0.01),qf_n));

    clear ssq_i
    clear psnrs_i
    clear entropys_i
    clear e
    clear qf
    
    load([directory 'entropys&psnrs_' image.name '_big'])

    ssq_12 = (255*10.^(-psnrs_12/20)).^2;
    ssq_34 = (255*10.^(-psnrs_34/20)).^2;
    ssq_5 = (255*10.^(-psnrs_5/20)).^2;
    ssq_m = (255*10.^(-psnrs_m/20)).^2;

%     ssq_34_mat = reshape(ssq_34,    [qf_n 1 1 n_it_qf n_it_nz]);
%     ssq_5_mat = reshape(ssq_5,      [qf_n 1 1 1 1 n_it_qf]);
%     ssq_i = ;
%     psnrs_i = 20*log10(256./sqrt(reshape(ssq_m(qf_i) + ssq_12(qf_i,) + ssq_34_mat + ssq_5_mat,qf_n,[])));
%     clear ssq_i

%     entropys_34_mat = reshape(entropys_34,   [qf_n 1 1 n_it_qf   n_it_nz]);
%     entropys_5_mat = reshape(entropys_5,     [qf_n 1 1 1      1       n_it_qf]);
%     entropys_i = reshape(entropys_m + entropys_12 + entropys_34_mat + entropys_5_mat,qf_n,[]);

    n_p = 100;
%     p_m = min(psnrs_0);
%     p_M = max(psnrs_0);
%     h = waitbar(0, 'Bounds determination');
%     for i_plot = 1:prod(dimensions)
%        if mod(i_plot,1000)==0
%            waitbar(i_plot/prod(dimensions))
%        end
%        cand_min = min(psnrs_i([1 qf_n],i_plot, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5));
%        if cand_min>p_m
%            p_m = cand_min;
%        end
%        cand_max = max(psnrs_i([1 qf_n],i_plot, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5));
%        if cand_max<p_M
%            p_M = cand_max;
%        end
%     end
%     p_m = max(min(psnrs_0), ...
%         std_to_psnr( sqrt( max(ssq_m.')+min(max(ssq_12(:,:).'))+min(max(ssq_34(:,:).'))+min(max(ssq_5.')) ) ));
%     p_M = min(max(psnrs_0), ...
%         std_to_psnr( sqrt( min(ssq_m.')+max(min(ssq_12(:,:).'))+max(min(ssq_34(:,:).'))+max(min(ssq_5.')) ) ));
    
    p_m = 28.586;
    p_M = 45.627;
%     close(h);

    p_v = linspace(p_m,p_M,n_p).';
    e_0 = interp1(psnrs_0,entropys_0,p_v,'pchip');
%     e = zeros(n_p, prod(dimensions));
%     qf = zeros(n_p, prod(dimensions));
    h = waitbar(0, 'Interpolation and Choice');
    minabs = Inf(n_p,1);
    Imin = nan(n_p,1);
    tic
    for i_plot = 1:347000*2 %prod(dimensions)
        if mod(i_plot,10000)==0
           waitbar(i_plot/prod(dimensions))
        end
%         e(:,i_plot) = interp1(psnrs_i( 1:qf_n, i_plot, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 ),...
%             entropys_i( 1:qf_n, i_plot, n_it_qf, n_it_nz, entropys_m, entropys_12, entropys_34, entropys_5 ),...
%             p_v,'pchip');
%         qf(:,i_plot) = interp1(psnrs_i( 1:qf_n, i_plot, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 ),...
%             qf_list(:),p_v,'pchip');
        e = interp1(psnrs_i( 1:qf_n, i_plot, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 ),...
            entropys_i( 1:qf_n, i_plot, n_it_qf, n_it_nz, entropys_m, entropys_12, entropys_34, entropys_5 ),...
            p_v,'linear');
        
        ind_new_min = e<minabs;
        minabs(ind_new_min)=e(ind_new_min);
        Imin(ind_new_min) = i_plot;
    end
    toc
    close(h);
%     clear psnrs_i
%     clear entropys_i
    
%     load([directory 'parameters_' image.name]);

%     [Ma,I] = min(e,[],2);
    M = minabs./e_0*100;
    [i1,i2,i3,i4,i5] = ind2sub(dimensions,Imin);
    qf_extension_1 = qf_extension_1_ar(i1).';
    null_zone_extension_1 = null_zone_extension_1_ar(i2).';
    qf_extension_2 = qf_extension_2_ar(i3).';
    null_zone_extension_2 = null_zone_extension_2_ar(i4).';
    qf_extension_3 = qf_extension_3_ar(i5).';
    
%     qf_LL = qf(sub2ind(size(qf),(1:n_p).',I));
    qf_LL = zeros(n_p,1);
    for i_p=1:n_p
%         qf_LL(i_p) = interp1(psnrs_i( 1:qf_n, Imin(n_p), n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 ),...
%             qf_list(:),p_v(i_p),'pchip');
        qf_LL(i_p) = interp1(psnrs_i( 1:qf_n, sub2ind(dimensions, i1(i_p),i2(i_p),i3(i_p),i4(i_p),i5(i_p)), n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 ),...
            qf_list(:),p_v(i_p),'pchip');
    end

    save([directory 'parameters_' image.name '_big'],'null_zone_extension_1','null_zone_extension_2','qf_extension_1','qf_extension_2','qf_extension_3','p_v','M','qf_LL');

    figure
    hold on;
    set(gcf,'Position', [400 200 1025 650]);
    plot(p_v, M);
end    
% return; 

% for im_num = 1:7
%     image = images(im_num);
%     load([directory 'parameters_' image.name])
% 
%     s_v = psnr_to_std(p_v);
%     dd_LL = 1./qf_LL;
% 
%     [sigma_qe1{im_num}, gof1] = fit(s_v, qf_extension_1.',fittype({'1+0*x'}));
%     [sigma_qe2{im_num}, gof2] = fit(s_v, qf_extension_2.',fittype({'1','x'}));
%     [sigma_qe3{im_num}, gof3] = fit(s_v, qf_extension_3.',fittype({'1+0*x'}));
%     [sigma_nze1{im_num}, gof4] = fit(s_v, null_zone_extension_1.',fittype({'1+0*x'}));
%     [sigma_nze2{im_num}, gof5] = fit(s_v, null_zone_extension_2.',fittype({'1','sqrt(x)'}));
%     
%     LinearModelTerms = {'1','x','x.^2','x.^3'};
%     afittype = fittype(LinearModelTerms);
%     dd_sigma{im_num}=fit(dd_LL,s_v,afittype);
% 
% end
% return;
    
for i_f = 1:17
    figure(i_f)
    set(gcf,'Position', [400 200 1025 650]);
%     cla
    hold on
    if i_f<=5
        xlabel('СКО')
    elseif i_f<=10
        xlabel('Шаг квантования LL')
    elseif i_f==11
        
    elseif i_f<=16
        xlabel('PSNR, dB')
    elseif i_f==17
        
    end
end

for im_num = 1
    image = images(im_num);
    load([directory 'parameters_' image.name '_big'])

    s_v = psnr_to_std(p_v);
    dd_LL = 1./qf_LL;

    figure(1)
    plot(s_v, qf_extension_1,'DisplayName',image.name)
    title('qf extension_1');ylabel('Коэффициент сужения зон первого уровня')
    figure(2)
    plot(s_v, qf_extension_2,'DisplayName',image.name)
    title('qf extension_2');ylabel('Коэффициент сужения зон второго уровня')
    figure(3)
    plot(s_v, qf_extension_3,'DisplayName',image.name)
    title('qf extension_3');ylabel('Коэффициент сужения зон третьего уровня')
    figure(4)
    plot(s_v, null_zone_extension_1,'DisplayName',image.name)
    title('null zone extension_1');ylabel('Коэффициент расширения нуль-зоны первого уровня')
    figure(5)
    plot(s_v, null_zone_extension_2,'DisplayName',image.name)
    title('null zone extension_2');ylabel('Коэффициент расширения нуль-зоны второго уровня')
    
    figure(6)
    plot(dd_LL, qf_extension_1,'DisplayName',image.name)
    title('qf extension_1')
    figure(7)
    plot(dd_LL, qf_extension_2,'DisplayName',image.name)
    title('qf extension_2')
    figure(8)
    plot(dd_LL, qf_extension_3,'DisplayName',image.name)
    title('qf extension_3')
    figure(9)
    plot(dd_LL, null_zone_extension_1,'DisplayName',image.name)
    title('null zone extension_1')
    figure(10)
    plot(dd_LL, null_zone_extension_2,'DisplayName',image.name)
    title('null zone extension_2')
    figure(11);xlabel('Шаг квантования');ylabel('PSNR, dB');
    plot(dd_LL,p_v,'-','DisplayName',image.name);
    
    figure(12)
    plot(p_v, qf_extension_1,'DisplayName',image.name)
    title('qf extension_1');ylabel('Коэффициент сужения зон первого уровня')
    figure(13)
    plot(p_v, qf_extension_2,'DisplayName',image.name)
    title('qf extension_2');ylabel('Коэффициент сужения зон второго уровня')
    figure(14)
    plot(p_v, qf_extension_3,'DisplayName',image.name)
    title('qf extension_3');ylabel('Коэффициент сужения зон третьего уровня')
    figure(15)
    plot(p_v, null_zone_extension_1,'DisplayName',image.name)
    title('null zone extension_1');ylabel('Коэффициент расширения нуль-зоны первого уровня')
    figure(16)
    plot(p_v, null_zone_extension_2,'DisplayName',image.name)
    title('null zone extension_2');ylabel('Коэффициент расширения нуль-зоны второго уровня')

    figure(17);ylabel('Шаг квантования');xlabel('PSNR, dB');
    plot(p_v, dd_LL,'-','DisplayName',image.name);
end

for im_num = 1
    image = images(im_num);
    load([directory 'parameters_' image.name '_big'])

    s_v = psnr_to_std(p_v);
    dd_LL = 1./qf_LL;

    figure(1)
     [sigma_qe1{im_num}, gof1] = fit(s_v, qf_extension_1(:),fittype({'1+0*x'}) );
%     disp(gof1.rmse)
    plot(s_v,sigma_qe1{im_num}(s_v),'--');
    
    figure(2)
    [sigma_qe2{im_num}, gof2] = fit(s_v, qf_extension_2(:),fittype({'1+0*x'}) );
%     disp(gof2.rmse)
    plot(s_v,sigma_qe2{im_num}(s_v),'--');
    
    figure(3)
     [sigma_qe3{im_num}, gof3] = fit(s_v, qf_extension_3(:),fittype({'1+0*x'}));
%     disp(gof3.rmse)
    plot(s_v,sigma_qe3{im_num}(s_v),'--');

    figure(4)
     [sigma_nze1{im_num}, gof4] = fit(s_v, null_zone_extension_1(:),fittype({'1','sqrt(x)'}));
%     disp(gof4.rmse)
    plot(s_v,sigma_nze1{im_num}(s_v),'--');

    figure(5)
    [sigma_nze2{im_num}, gof5] = fit(s_v, null_zone_extension_2(:),fittype({'1','x'}));
%     disp(gof5.rmse)
    plot(s_v,sigma_nze2{im_num}(s_v),'--');

    dd_LL_v = linspace(5,55);
%    
    LinearModelTerms = {'1','x','x.^2','x.^3'};
    afittype = fittype(LinearModelTerms);
    dd_sigma{im_num}=fit(dd_LL,s_v,afittype);
    
    figure(6)
    plot(dd_LL_v,sigma_qe1{im_num}(dd_sigma{im_num}(dd_LL_v)),'--');
    
    figure(7)
    plot(dd_LL_v,sigma_qe2{im_num}(dd_sigma{im_num}(dd_LL_v)),'--');
    
    figure(8)
    plot(dd_LL_v,sigma_qe3{im_num}(dd_sigma{im_num}(dd_LL_v)),'--');

    figure(9)
    plot(dd_LL_v,sigma_nze1{im_num}(dd_sigma{im_num}(dd_LL_v)),'--');

    figure(10)
    plot(dd_LL_v,sigma_nze2{im_num}(dd_sigma{im_num}(dd_LL_v)),'--');   
    
    figure(11)
    plot(dd_LL_v,std_to_psnr(dd_sigma{im_num}(dd_LL_v)),'--');
    

    figure(12)
     [sigma_qe1{im_num}, gof1] = fit(s_v, qf_extension_1(:),fittype({'1+0*x'}) );
%     disp(gof1.rmse)
    plot(p_v,sigma_qe1{im_num}(s_v),'--');
    
    figure(13)
    [sigma_qe2{im_num}, gof2] = fit(s_v, qf_extension_2(:),fittype({'1+0*x'}) );
%     disp(gof2.rmse)
    plot(p_v,sigma_qe2{im_num}(s_v),'--');
    
    figure(14)
     [sigma_qe3{im_num}, gof3] = fit(s_v, qf_extension_3(:),fittype({'1+0*x'}));
%     disp(gof3.rmse)
    plot(p_v,sigma_qe3{im_num}(s_v),'--');

    figure(15)
     [sigma_nze1{im_num}, gof4] = fit(s_v, null_zone_extension_1(:),fittype({'1','sqrt(x)'}));
%     disp(gof4.rmse)
    plot(p_v,sigma_nze1{im_num}(s_v),'--');

    figure(16)
    [sigma_nze2{im_num}, gof5] = fit(s_v, null_zone_extension_2(:),fittype({'1','x'}));
%     disp(gof5.rmse)
    plot(p_v,sigma_nze2{im_num}(s_v),'--');

    figure(17)
    plot(std_to_psnr(dd_sigma{im_num}(dd_LL_v)), dd_LL_v,'--');
    
end

directory = '/home/gleb/Documents/WaveletDecomposition/WaveletDecomposition/Graphs/Quant/Optimum/Parameters/';
for i_f = 1:17
    figure(i_f)
    legend show
    figname = [directory 'im' num2str(i_f) '.fig'];
    bmpname = [directory 'im' num2str(i_f) '.bmp'];
    save(figname);
    saveas(gcf, bmpname);
end

return


plot(p_v,e./e_0*100);
if ~load_plots
    plot(p_v,100*ones(1,n_p),'k--');
end
e2 = reshape(e./e_0*100, numel(iter1_ar), numel(iter2_ar));
[I1, I2] = meshgrid(iter1_ar, iter2_ar);
mesh(I1,I2,e2);
xlabel('iter1')
ylabel('iter2')
legend({'[1] Median(10) shift of 3L, 5x5', '[2] 1.5 null-zone of 2H', 'Both'});
legend({'[1] Median(10) shift of 3L, 5x5', '[2] 1.5 null-zone of 2H', '[3] 1.5 null-zone of 3H', '[1]+[2]', '[1]+[3]'});
legend({'Сдвиг НЧ 3 уровня на медиану в блоках 5x5', 
    'x1.5 нуль-зона ВЧ 1 уровня', 
    'x1.5 нуль-зона ВЧ 2 уровня',
    'Совокупное сжатие'});
legend(num2str((1:size(psnrs_i,2)).'))
xlabel('PSNR, dB');
ylabel('Relative entropy, %');
title(titlelabel);