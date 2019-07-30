in_dir = 'output/output_last/files/';
out_dir = 'output/output_last/distributions/';
comp = 'Y';
image = 'kiel.bmp';
qs = 0.03;
bands=1:3;  % numerating from 0

fname = [in_dir 'bands_' comp '_'  image '_' sprintf('%8f',qs) '.dat'];
fd = fopen(fname, 'rb');
assert(fd~=-1);

figure
hold on;
left_common = -Inf;
right_common = +Inf;
for i=1:max(bands)+1
    wh = fread(fd, 2, 'int');
    size = prod(wh);
    data = fread(fd, size, 'short');
    if any((bands+1)==i)
%         ksdensity(data,linspace(min(data),max(data),1000));
        left = quantile(data,0.01);
        right = quantile(data,0.99);
        left_common = max(left_common, left);
        right_common = min(right_common, right);
        ksdensity(data,linspace(left,right,1000), 'bandwidth',[]);
    end
end

xlim([left_common, right_common]);
legend(num2str(bands.'));
% set(gcf,'Position', [200 200 1025 650]);
fclose(fd);

foutname = [out_dir 'distr_' comp '_'  image '_' sprintf('%8f',qs) '.bmp'];
saveas(gcf,foutname);