bandc = {'LL', 'ML', 'HL', 'LM', 'MM', 'HM', 'LH', 'MH', 'HH'};
for it = 1:8
pow = any(it==[0 4 8])+1;
band = bandc{it+1};
norig = 642;

add = false;
conj_add = false;
Nlog=norig+it;
Nbands=1;
grapher

conj_add = true;
Nlog=705+it;
Nbands=2^pow;
leg_orig = string(['Division - ' num2str(Nbands)]);
markerstyle_orig = '-o';
grapher

add = true;
conj_add = false;
Nlog=norig+it;
Nbands=1;
grapher

conj_add = true;
Nlog=714+it;
Nbands=3^pow;
leg_orig = string(['Division - ' num2str(Nbands)]);
markerstyle_orig = '-o';
grapher

conj_add = false;
Nlog=norig+it;
Nbands=1;
grapher

conj_add = true;
Nlog=723+it;
Nbands=1^pow;
leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
markerstyle_orig = '-.o';
grapher

conj_add = false;
Nlog=norig+it;
Nbands=1;
grapher

conj_add = true;
Nlog=732+it;
Nbands=2^pow;
leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
markerstyle_orig = '-.o';
grapher

conj_add = false;
Nlog=norig+it;
Nbands=1;
grapher

conj_add = true;
Nlog=741+it;
Nbands=3^pow;
leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
markerstyle_orig = '-.o';
grapher

end



% for it = 3:8
% pow = any(it==[4 8])+1;
% band = bandc{it+1};
% 
% add = false;
% conj_add = false;
% Nlog=696+it;
% Nbands=1;
% leg_orig = string(['Division - ' num2str(Nbands)]);
% markerstyle_orig = '-o';
% grapher
% 
% add = true;
% Nlog=705+it;
% Nbands=2^pow;
% leg_orig = string(['Division - ' num2str(Nbands)]);
% grapher
% 
% Nlog=714+it;
% Nbands=3^pow;
% leg_orig = string(['Division - ' num2str(Nbands)]);
% grapher
% 
% 
% Nlog=723+it;
% Nbands=1;
% leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
% markerstyle_orig = '-.o';
% grapher
% 
% Nlog=732+it;
% Nbands=2^pow;
% leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
% grapher
% 
% Nlog=741+it;
% Nbands=3^pow;
% leg_orig = string(['Division (A/S) - ' num2str(Nbands)]);
% grapher
% 
% 
% Nlog=642+it;
% Nbands=1;
% markerstyle_orig = '--o';
% leg_orig = string(['Orig, no context']);
% grapher
% 
% end
