n=2;
quant_bp
% figure(1);
% % contourf(component)
% mesh(20*log10(abs(component)))
comp_ext = zeros(size(component)*4);
comp_ext((1:size(component,1)),(1:size(component,2)))=component;
F=fft2(comp_ext);
% figure(2);
% mesh(abs(F));
% % mesh(20*log10(abs(F)));

ce=reshape(component_error(:,45),size(component));
% ce=ce-mean(ce(:));
% Fce=fft2(ce);
% contourf(abs(F))
% figure(3);
% mesh(20*log10(abs(Fce)))

er_corr = xcorr2(ce)./xcorr2(ones(size(ce)))/sum(ce(:).^2)*size(ce,1)*size(ce,2);
% figure(4);
% [xc, yc] = size(ce);
% surf(10*log10(abs(er_corr(xc:xc+50,yc:yc+50))));
% xlabel('x');
% ylabel('y');
% zlabel('Correlation, dB');

% figure(5)
% plot(sum(ce.^2,1))
% figure(6)
% plot(sum(ce.^2,2))
% plot(ce(:,50))