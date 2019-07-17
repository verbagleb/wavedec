fid = fopen('config-2odd.cfg');
for i=1:13
    fgetl(fid);
end
n_filters = fscanf(fid,'%d');
for k=0:n_filters-1
    fgetl(fid);
    data.name(k+1) = string(fgetl(fid));
    data.index(k+1) = k;
    for i=1:7
        fgetl(fid);
    end
    a1=str2num(fgetl(fid)); 
    a2=str2num(fgetl(fid));
    a1(numel(a1)+1:max(numel(a1),numel(a2)))=0;
    a2(numel(a2)+1:max(numel(a1),numel(a2)))=0;
    a=[a1;a2];
    a=[a(:,end:-1:2).*[1; 1j] a(:,2:end).*[1; -1j]];
%     syms x;
    Ax=@(x)real(a*(exp(1j*pi*x(:)).^(-size(a,2)/2+1/2:size(a,2)/2-1/2)).');
    Ax_abs=@(x)abs(prod(Ax(x)));
    Ax_sq=@(x)(prod(Ax(x))).^2;
    data.int_abs(k+1) = integral(Ax_abs,-1,1);
    data.int_sq(k+1) = integral(Ax_sq,-1,1);
%     Bx=Bf*exp(1j*pi*x).^(-size(B,2)+1/2:size(B,2)-1/2).';
    for i=1:3
        fgetl(fid);
    end
end
fclose(fid);

disp('Abs');
[B,I]=sort(data.int_abs);
disp([data.index(I).', data.name(I).', data.int_abs(I).']);

disp('Square');
[B,I]=sort(data.int_sq);
disp([data.index(I).', data.name(I).', data.int_sq(I).']);