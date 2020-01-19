    
% Smooth the data
    %Mass
    mass_Med_Smooth = smoothdata(data.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(data.MassRate, 'movmedian', 80)
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(data.Freq, 'movmedian', 10)
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(data.TowerT, 'movmedian', 5)
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(data.RoomT, 'movmedian', 5)
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

%% Plotting
    %Frequency/Mass/Resistivity vs Time
        close all
        figure
        hold on
        title 'Frequency, Mass, Temperature'
        xlim([-1000  21000])
        xlabel 'Time'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([120000 150000])
            %ylim([0 5000000])
            plot(freq_Smooth, '-b')
            %plot(resistivity_Smooth/5, 'g')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Temperature'
            ylim([0 125])
            plot(mass_Smooth/25, '-m')
            plot(tower_Temp_Smooth, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            plot(mass_Rate_Smooth*100, 'y')
         
        % Legend 
            legend({'Frequency','Mass/15', 'Tower Temperature', 'Wash Temperature','Mass Rate * 100' },'Location','Northwest')
            hold off

    %Frequency Vs Mass
        figure
        hold on
        title 'Frequency vs Mass'
        xlim([0  1600])
        xlabel 'Mass'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([108000 144000])
            plot(mass_Smooth, freq_Smooth, '-b')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Tower Temp'
            ylim([50 120])
            plot(mass_Smooth, tower_Temp_Smooth, '-r')

        %Vertical Lines
            xl = xline(80,'--k');
            xl.LabelVerticalAlignment = 'middle';
            xl = xline(1495,'--k');
            xl.LabelVerticalAlignment = 'middle';

 legend({'Frequency', 'Tower Temp'},'Location','Northeast')
 hold off
