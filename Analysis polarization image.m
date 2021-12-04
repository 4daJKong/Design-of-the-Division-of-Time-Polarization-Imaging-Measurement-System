%Polarization image analysis based on Stokes%
S1=imread('0.bmp'); %read 0 degree polarization image
S2=imread('45.bmp');%read 45 degree polarization image
S3=imread('90.bmp');%read 90 degree polarization image
S4=imread('135.bmp');%read 135 degree polarization image
D0=im2double(S1);
D45=im2double(S2);
D90=im2double(S3);
D135=im2double(S4);
I = 1 / 2 * (D0 + D45 + D90 + D135);%I:Total intensity of light wave
Q = (D0 - D90);%Horizontally polarized light intensity
U =(D45 - D135);%Intensity of linearly polarized light in 45 degree direction

DOP = sqrt(Q.*Q + U.*U)./I; %Degree of polarization
AOP = atan(U./Q) / 2;%Polarization azimuth


