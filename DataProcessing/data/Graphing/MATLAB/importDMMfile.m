function DMM = importDMMfile(filename, dataLines)
%IMPORTFILE Import data from a text file
%  DMM = IMPORTDMMFILE(FILENAME) reads data from text
%  file FILENAME for the default selection.  Returns the data as a table.
%
%  DMM = IMPORTDMMFILE(FILE, DATALINES) reads data for
%  the specified row interval(s) of text file FILENAME. Specify
%  DATALINES as a positive scalar integer or a N-by-2 array of positive
%  scalar integers for dis-contiguous row intervals.
%
%  Example:
%  DMM = importDMMfile(filename, [1, Inf]);
%
%  See also READTABLE.

%% Input handling

% If dataLines is not specified, define defaults
if nargin < 2
    dataLines = [1, Inf];
end

%% Setup the Import Options and import the data
opts = delimitedTextImportOptions("NumVariables", 2);

% Specify range and delimiter
opts.DataLines = dataLines;
opts.Delimiter = "\t";

% Specify column names and types
opts.VariableNames = ["Time", "Resistance"];
opts.VariableTypes = ["datetime", "double"];

% Specify file level properties
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";

% Specify variable properties
opts = setvaropts(opts, "Time", "InputFormat", "");

% Import the data
DMM = readtable(filename, opts);

end