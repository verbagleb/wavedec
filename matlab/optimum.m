clear;

Nlog = 846;
Nbands = 4;
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
quant_factor(:,1) = Yid.data(:,3);
Y.Extra = Yid.data(:,6+3*Nbands);
sigma_sq(:,1)=10.^(-Y.Extra/10);


Nlog = 847;
Nbands = 4;
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
quant_factor(:,2) = Yid.data(:,3);
Y.Extra = Yid.data(:,6+3*Nbands);
sigma_sq(:,2)=10.^(-Y.Extra/10);

Nlog = 848;
Nbands = 4;
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
quant_factor(:,3) = Yid.data(:,3);
Y.Extra = Yid.data(:,6+3*Nbands);
sigma_sq(:,3)=10.^(-Y.Extra/10);

Nlog = 849;
Nbands = 4;
Yid = importdata(['log/log_', num2str(Nlog), '/Y_log_short.txt']);
quant_factor(:,4) = Yid.data(:,3);
Y.Extra = Yid.data(:,6+3*Nbands);
sigma_sq(:,4)=10.^(-Y.Extra/10);

bitmap=Yid.textdata(2:end,1);
filter = Yid.data(:,1);
entropy = Yid.data(:,4:2:2+2*Nbands);

restrict_filter = 17;
restrict_bitmap = "bitmap_4.bmp";
cond = filter==restrict_filter & bitmap==restrict_bitmap;
hold off;

entropy_f=cell(1,4);
quant_factor_f=cell(1,4);
% cond_no_rep = ([1; diff(sigma_sq(cond,1))]~=0);
for i=1:4
    [sigma_sq_r, i_or, i_r] = unique(sigma_sq(cond,i));
    entropy_r = entropy(cond,i);
    entropy_r = entropy_r(i_or);
    quant_factor_r = quant_factor(cond,i);
    quant_factor_r = quant_factor_r(i_or);
    entropy_f{i}=@(s_sq)interp1(sigma_sq_r, entropy_r, s_sq, 'pchip', NaN);
    quant_factor_f{i}=@(s_sq)interp1(sigma_sq_r, quant_factor_r, s_sq, 'pchip', NaN);
end
entropy_s_f = @(s_sq_v) ...
    entropy_f{1}(s_sq_v(1)) + ...
    entropy_f{2}(s_sq_v(2)) + ...
    entropy_f{3}(s_sq_v(3)) + ...
    entropy_f{4}(s_sq_v(4));
quant_factor_s_f = @(s_sq_v) [
    quant_factor_f{1}(s_sq_v(1)),  ...
    quant_factor_f{2}(s_sq_v(2)),  ...
    quant_factor_f{3}(s_sq_v(3)),  ...
    quant_factor_f{4}(s_sq_v(4))
    ];
sigma_sq_min=min(sigma_sq(cond,:));
sigma_sq_max=max(sigma_sq(cond,:));
% sigma_sq_t = linspace(min(sigma_sq(cond,1)), max(sigma_sq(cond,1)),1000000);
% plot(sigma_sq_t,interp1(sigma_sq_r, entropy_r, sigma_sq_t, 'pchip'));
% hold on;
% plot(sigma_sq(cond,1),entropy(cond,1),'o');
% hold off;
% plot(sigma_sq_t, quant_factor_f{1}(sigma_sq_t));
% hold on;
% plot(sigma_sq(cond,:),quant_factor(cond,:),'o');

psnr_v = linspace(20,60,100);
s_sq_min = zeros(numel(psnr_v),4);
qf_min = zeros(numel(psnr_v),4);
ent_v = psnr_v*0;
for i=1:numel(psnr_v)
    psnr = psnr_v(i);
    s_sq=10.^(-psnr/10);
    % t=linspace(-5e-5,5e-5);
    % ent=zeros(size(t));
    % for i=1:numel(t)
    %     ent(i)=entropy_s_f([s_sq/4-t(i) s_sq/4+t(i) s_sq/4-t(i) s_sq/4+t(i)]);
    % end
    % plot(t,ent);
    init = ones(4,1)*s_sq/4;
%     init = [s_sq/3 s_sq/4 s_sq/4 s_sq/6];
    [s_sq_min(i,:), ent_v(i)] = fmincon(entropy_s_f,init,[],[],ones(1,4), s_sq, sigma_sq_min, sigma_sq_max);
    qf_min(i,:) = quant_factor_s_f(s_sq_min(i,:));
end
figure
xd=sum(entropy(cond,:),2);
[yd, ix, iy]=unique(-10*log10(sum(sigma_sq(cond,:),2)));
xd=xd(ix);
plot(xd,yd);
hold on;
plot(ent_v,psnr_v);
hold off;
ylim([20 60]);
xlabel('Entropy, bits');
ylabel('PSNR, dB');
figure
plot(ent_v.',qf_min);
xlabel('Entropy, bits');
ylabel('quant_factor');
