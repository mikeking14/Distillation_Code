    mass_Med_Smooth = smoothdata(arduino.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(arduino.Mass_Delta, 'movmedian', 80);
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(arduino.Frequency, 'movmedian', 10);
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(arduino.Tower_Temp, 'movmedian', 5);
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(arduino.Wash_Temp, 'movmedian', 5);
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

    outlet_Temp_Med_Smooth = smoothdata(arduino.Outlet_Temp, 'movmedian', 5);
    outlet_Temp_Smooth = smoothdata(outlet_Temp_Med_Smooth, 'movmean', 10);
    
    
    %% Plotting
    %Frequency/Mass/Resistivity vs Time
        close all
        figure
        hold on
        title 'Frequency, Mass, Temperature'
        xlabel 'Time'

        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([120000 170000])
            %ylim([0 5000000])
            plot(freq_Smooth, '-b')
            %plot(resistivity_Smooth/5, 'g')
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Temperature'
            ylim([0 125])
            plot(arduino.Mass/15, '-m')
            plot(arduino.SetTemp, '--r')
            plot(arduino.Tower_Temp, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            plot(mass_Rate_Smooth*100, '-y')
         
        % Legend 
            legend({'Frequency','Mass/15', 'Set Temperature', 'Tower Temperature','Wash Temp','Outlet Temp', 'Mass Rate * 100' },'Location','Northwest')
            hold off

            
     %% Plotting 2
        close all
        figure
        hold on
        title 'PID CONTROL'
        xlabel 'Time'
        
        
        yyaxis left
        ylabel 'Temperature'
        plot(arduino.SetTemp, '--red')
        plot(arduino.Tower_Temp, '-red')


        yyaxis right
        ylabel 'PID Variables'
        ylim([-100 300])
        plot(arduino.PID)
        plot(arduino.PID_Error,'black')
        plot(arduino.P, '-m')
        plot(arduino.I, '-b')
        plot(arduino.D, 'c')
        
        legend({'Set Temp','Tower Temp', 'PID', 'PID ERROR','P','I', 'D' },'Location','Northwest')

