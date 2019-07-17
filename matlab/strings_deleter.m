% please be careful
% doesn't work

Nlog=1082;   % number of log set
Nbands=9;   % number of bands in log
            
% split_size_code=100;    %indicates usage of split with (block_size-split_size_code)
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
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

