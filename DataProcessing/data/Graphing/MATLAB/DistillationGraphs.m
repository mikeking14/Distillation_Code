clear; close all; clc;
%% Initialize Workspace
    runNumber = 49;
    
    methanol = 320; 
    IPA = 1259;
%% Import Arduino Data
        
    % Specify Current Directory      
    s1 = pwd;
    s1 = s1(1:size(s1,2)-15);
    % Specify Arduino data file location
    s2 = sprintf('Run%d/DistillationRun%d_Arduino.txt',runNumber,runNumber);
    Arduinofilename = horzcat(s1,s2);
    % Import Arduino data
    arduino = importArduinoFile(Arduinofilename);
        

%% Import DMM Data
       
    % Specify DMM data file location
    s2 = sprintf('Run%d/DistillationRun%d_DMM.txt',runNumber,runNumber);
    DMMfilename = horzcat(s1,s2);
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

    mass_Rate_Med_Smooth = smoothdata(data.Mass_Delta, 'movmedian', 800);
    mass_Rate_Smooth = smoothdata(mass_Rate_Med_Smooth, 'movmean', 60);

    %Frequency
    freq_Med_Smooth = smoothdata(data.Frequency, 'movmedian', 10);
    freq_Smooth = smoothdata(freq_Med_Smooth, 'movmean', 30);

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
            ylim([3000 900000])
            %plot(freq_Smooth, '-b')
            plot(resistivity_Smooth, 'g')
                
                % Vetical Line where Methanol/IPA to be expected
                [d,i_IPA] = min( abs( data.Mass-methanol ) );
                [d,i_H20] = min( abs( data.Mass-IPA) );
                %xline(i_IPA);
                %xline(i_H20);
                            
            % Right side of graph
            yyaxis right
            ylim([-10 120]);
            ylabel 'Temperature (degrees)'
            plot(data.Mass/50, '-m')
            plot(data.Set_Temp, '--r')
            plot(data.Tower_Temp, '-r')
            plot(data.Room_Temp, '-c')
            plot(data.Outlet_Temp, '-k')
            p1 = plot(mass_Rate_Smooth*100, '-y');
            p1.Color(4) = 0.8;

            %text(i_IPA,max(data.Tower_Temp+20),'\leftarrow Methanol Gone')
            %text(i_H20,max(data.Tower_Temp+15),'\leftarrow IPA Gone')
         
        % Legend 
            legend({'Resistivity', 'Mass/50', 'Set Temp','Tower Temperature', 'Wash Temperature','Outlet Temp','Mass Rate * 100' },'Location','Northeast')
            hold off

            
%% Plotting - Frequency Vs Mass

        figure
        hold on
        title '(Resistance + Frequency) vs Mass'
        xlabel 'Mass'

        % Left side of graph
            yyaxis left
            ylabel 'Resistivity + Frequency'
            ylim([100000 200000])
            plot(mass_Smooth, resistivity_Smooth, '-b')
            plot(mass_Smooth, freq_Smooth, '-black')
        
        % Right side of graph
            yyaxis right
            ylabel 'Mass / Tower Temp'
            ylim([50 120])
            plot(mass_Smooth, data.Tower_Temp, '-r')
            
        % Legend
            legend({'Resistivity', 'Frequency', 'Tower Temp'},'Location','Northeast')
            hold off

            
%% Resistance Vs Outlet Temp Graphs

        figure
        hold on
        title 'Resistance & Outlet Temperature'
        xlabel 'Time'
       
        
        % Left side of graph
            yyaxis left
            ylabel 'Resistivity'
            ylim([0 400e3])

            plot(data.Resistance, '-b')
            
      % Right side of graph
            yyaxis right
            ylabel 'Outlet Temp'
            plot(data.Outlet_Temp, '-r')
            
legend({'Resistance', 'Outlet Temp'},'Location','Northeast')


%% Resistance Graphs Again

        figure
        hold on
        title 'All Data'
        xlabel 'Time'
        
        % Left side of graph
            yyaxis left
            ylabel 'Resistivity'
            ylim([0 100000])
            plot(resistivity_Smooth, 'g')
            plot(data.Mass*25, '-m')

            [d,i_IPA] = min( abs( data.Mass-320 ) )
            [d,i_H20] = min( abs( data.Mass-1259) )

            xline(i_IPA)
            xline(i_H20)
            yline(0)


