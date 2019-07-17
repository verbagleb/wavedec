function [ dwt_ca ] = dwt( array, dir_filter, shift )
%Direct DWT
%   Detailed explanation goes here

    N = size(dir_filter, 1);
    dir_len=size(dir_filter,2);
    dir_len_h = (dir_len-1)/2;
    extension = dir_len_h;

    [h_or, w_or]=size(array);
    h_c=deal( ceil( (h_or-1)/N )*N+1 );
    w_c=deal( ceil( (w_or-1)/N )*N+1 );
    original_c=array-shift;
    for h = h_or+1:h_c
        original_c(end+1,:)=original_c(end,:);
    end
    for w = w_or+1:w_c
        original_c(:,end+1)=original_c(:,end);
    end
    original_ext = original_c([extension+1:-1:1+1, 1:end, end-1:-1:end-extension],[extension+1:-1:1+1, 1:end, end-1:-1:end-extension]);

    for n=1:N
        dwt_half_2ext{n}=conv2(original_ext,dir_filter(n,:));
        dwt_half_ext{n}=dwt_half_2ext{n}(:,1+extension+dir_len_h:end-(extension+dir_len_h));
        dwt_half_ext{n}=dwt_half_ext{n}(:,1:N:end);
        for m=1:N
            dwt_ext{m,n}=conv2(dwt_half_ext{n},dir_filter(m,:).');
            dwt_ca{m,n}=dwt_ext{m,n}(1+extension+dir_len_h:end-(extension+dir_len_h),:);
            dwt_ca{m,n}=dwt_ca{m,n}(1:N:end,:);
        end
    end

end

