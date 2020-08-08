
data = importdata('pointmap.txt');
xdata=data(:,1);ydata=data(:,2);zdata = data(:,3);

scatter3(xdata,ydata,zdata);
%length(xdata)
%xdata=-xdata;
% 显示深度图
%figure;
%depth=reshape(zdata,308,480);
%mesh(xdata);
% 显示重构的三维环境
%figure;
%[cx,cy] = meshgrid(min(xdata(:)):max(xdata(:)),min(ydata(:)):max(ydata(:)));
%cz=griddata(xdata,ydata,zdata,cx,cy,'cubic');
%mesh(cx,cy,cz);
%xlabel('Width');    ylabel('Height');   zlabel('Depth');