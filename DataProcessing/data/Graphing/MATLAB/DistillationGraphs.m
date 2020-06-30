clear; close all; clc;
%% Initialize Workspace
        runNumber = 42;

%% Import Arduino Data
        
        % Specify Filepath
        str = "/Users/michaelking/Documents/PlatformIO/Projects/Distillation_Code/DataProcessing/data/Run";
        str2 = num2str(runNumber);
        str3 = "/DistillationRun";
        str4 = "_Arduino.txt";
        
        % Import Arduino data
        Arduinofilename = append(str,str2,str3,str2,str4);
        arduino = importArduinoFile(Arduinofilename);
        

%% Import DMM Data
       
        % Specify Filepath
        str5 = "_DMM.txt";
        DMMfilename = append(str,str2,str3,str2,str5);
        % Import DMM data
        DMM = importDMMfile(DMMfilename);


%% Combine the Arduino and DMM data
    
    % Convert the arduino and DMM data into timetables
    arduino_Timetable = table2timetable(arduino);
    DMM_Timetable = table2timetable(DMM);
    
    % Gets rid of duplicate times in DMM... fix in labview then delete this
    DMM_Timetable = retime(DMM_Timetable,'secondly','mean');
    
    % Synchronize the two data steams into a timetable called data
    data = synchronize(arduino_Timetable,DMM_Timetable,'union', 'linear');
    
    % Combine mass
    mass = data.Mass;
    lagged_mass = zeros(size(mass));
    lagged_mass(2:end) = mass(1:end-1);
    change = mass - lagged_mass;
    change(abs(change) > 10) = 0;
    data.Mass = cumsum(change);
    
%% Smooth these data

    %Mass
    mass_Med_Smooth = smoothdata(data.Mass, 'movmedian', 30);
    mass_Smooth=smoothdata(mass_Med_Smooth,'movmean',10);

    mass_Rate_Med_Smooth = smoothdata(data.Mass_Delta, 'movmedian', 80);
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(data.Frequency, 'movmedian', 10);
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

    %Temperature
    tower_Temp_Med_Smooth = smoothdata(data.Tower_Temp, 'movmedian', 5);
    tower_Temp_Smooth = smoothdata(tower_Temp_Med_Smooth, 'movmean', 10);

    wash_Temp_Med_Smooth = smoothdata(data.Room_Temp, 'movmedian', 5);
    wash_Temp_Smooth = smoothdata(wash_Temp_Med_Smooth, 'movmean', 10);

    outlet_Temp_Med_Smooth = smoothdata(data.Outlet_Temp, 'movmedian', 15);
    outlet_Temp_Smooth = smoothdata(outlet_Temp_Med_Smooth, 'movmean', 25);

    resistivity_Med_Smooth = smoothdata(data.Resistance, 'movmedian', 20);
    resistivity_Smooth = smoothdata(resistivity_Med_Smooth, 'movmean', 10);

    
%% Plotting - All Data

        close all
        figure
        hold on
        title 'All Data'
        xlabel 'Time'
        
        % Left side of graph
            yyaxis left
            ylabel 'Frequency'
            ylim([126000 134000])
            plot(freq_Smooth, '-b')
            
        % Right side of graph
            yyaxis right
            ylim([-10 120]);
            ylabel 'Y'
            plot(resistivity_Smooth / 12000, 'g')
            plot(data.Mass/50, '-m')
            plot(data.Set_Temp, '--r')
            plot(data.Tower_Temp, '-r')
            plot(wash_Temp_Smooth, '-c')
            plot(outlet_Temp_Smooth, '-k')
            p1 = plot(mass_Rate_Smooth*100, '-y');
            p1.Color(4) = 0.25;
         
        % Legend 
            legend({'Frequency','Resistivity/ 1.5','Mass/50', 'Set Temp','Tower Temperature', 'Wash Temperature','Outlet Temp','Mass Rate * 100' },'Location','Northwest')
            hold off

% %% Plotting - Frequency Vs Mass
% 
%         figure
%         hold on
%         title 'Frequency vs Mass'
%         xlabel 'Mass'
% 
%         % Left side of graph
%             yyaxis left
%             ylabel 'Frequency'
%             ylim([108000 144000])
%             plot(mass_Smooth, freq_Smooth, '-b')
%         
%         % Right side of graph
%             yyaxis right
%             ylabel 'Mass / Tower Temp'
%             ylim([50 120])
%             plot(mass_Smooth, tower_Temp_Smooth, '-r')
%             
%         % Legend
%             legend({'Frequency', 'Tower Temp'},'Location','Northeast')
%             hold off
