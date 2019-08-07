in_dir = 'output/output_last/files/';
out_dir = 'output/output_last/distributions/';
TYPE = 'float';

comp = 'Y';
image = 'kiel.bmp';
bands=1:3;  % numerating from 0

band_names =  ["LL" "LH" "HL" "HH"];    % TODO: create name output

fname = [in_dir 'bands_' comp '_'  image '.dat'];
fd = fopen(fname, 'rb');
assert(fd~=-1);

figure
hold on;
left_common = -Inf;
right_common = +Inf;
for i=1:max(bands)+1
    wh = fread(fd, 2, 'int');
    size = prod(wh);
    data = fread(fd, size, TYPE);
    if any((bands+1)==i)
%         ksdensity(data,linspace(min(data),max(data),1000));
        left = quantile(data,0.01);
        right = quantile(data,0.99);
        left_common = max(left_common, left);
        right_common = min(right_common, right);
        [f, xi] = ksdensity(data,linspace(left,right,1000), 'bandwidth',[]);
        f0 = ksdensity(data,0, 'bandwidth',[]);
        plot(xi, f/f0);
    end
end

xlim([left_common, right_common]);
% legend(num2str(bands.'));
legend(band_names(bands+1));
% set(gcf,'Position', [200 200 1025 650]);
fclose(fd);

mkdir(out_dir);
foutname = [out_dir 'distr_' comp '_'  image '.bmp'];
saveas(gcf,foutname);