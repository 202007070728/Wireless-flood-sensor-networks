import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pyextremes import EVA
import scipy.stats as stats
import os

plt.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False


def comprehensive_pot_analysis(csv_file_path, threshold_quantile=0.99):
    """
    The complete POT analysis process
    """
    # 1. Read data
    print("step1: Read data...")

    csv_file_path = r"E:\daily_VP1_RF_ALL.csv"

    data = pd.read_csv(csv_file_path,
                       skiprows=3,  # Skip the first 3 lines of comments
                       usecols=[0, 1, 2, 3],  # Select columns A, B, C, D (year, month, day, value)
                       names=['Year', 'Month', 'Day', 'Rainfall_mm'])  # Specify column name

    data['Date'] = pd.to_datetime(data[['Year', 'Month', 'Day']])

    data.set_index('Date', inplace=True)

    rainfall = data['Rainfall_mm']

    # Check the basic situation of the data
    print(f"Data period: {rainfall.index[0]} to {rainfall.index[-1]}")
    print(f"Total number of records: {len(rainfall)}")
    print(f"Maximum rainfall: {rainfall.max():.1f} mm")
    print(f"Data preview:")
    print(rainfall.head(10))

    # 2. Data quality check
    print(f"\nData quality check:")
    print(f"The number of missing values: {rainfall.isnull().sum()}")
    print(f"Zero days without rainfall: {len(rainfall[rainfall == 0])}")
    print(f">100mm rainfall duration: {len(rainfall[rainfall > 100])}")
    print(f">200mm rainfall duration: {len(rainfall[rainfall > 200])}")

    # 3. Manually calculate the threshold
    threshold = np.percentile(rainfall, threshold_quantile * 100)
    print(f"\nThe used threshold (the {threshold_quantile * 100}% percentile): {threshold:.1f} mm")

    # 4. Extract the data that exceeds the threshold
    extremes = rainfall[rainfall > threshold]
    print(f"The number of events above the threshold: {len(extremes)}")
    print(f"Proportion of exceedance events: {len(extremes) / len(rainfall) * 100:.2f}%")

    # 5. Create an EVA object and conduct a POT analysis
    print("\nStep 2: Conduct POT analysis...")
    model = EVA(data=rainfall)

    # Use the revised parameters
    model.get_extremes(
        method="POT",
        threshold=threshold,  # Threshold can be directly specified
        r="24h"
    )

    # 6. Fit the generalized Pareto distribution
    print("\nStep 3: Fit the generalized Pareto distribution...")
    model.fit_model()

    # Obtain fitting parameters - Fix the issue of parameter unpacking
    excesses = extremes.values - threshold

    # Method 1: Use scipy for direct fitting, handling cases where the number of parameters is unknown
    params = stats.genpareto.fit(excesses)
    print(f"GPD fitting parameters: {params}")

    # Process dynamically based on the number of parameters
    if len(params) == 3:
        shape, loc, scale = params
        print(f"GPD fitting parameters - Shape parameter: {shape:.4f}, Location parameter: {loc:.4f}, Scale parameter: {scale:.4f}")
    elif len(params) == 2:
        # In some cases, only the shape and scale parameters may be returned.
        shape, scale = params
        loc = 0  # Assume that the position parameter is 0
        print(f"GPD fitting parameters - Shape parameters: {shape:.4f}, Scale parameters: {scale:.4f} (Position parameters are fixed at 0)")
    else:
        print(f"Unexpected number of parameters: {len(params)}")
        shape, loc, scale = params[0], 0, params[-1]

    # 7. Manually calculate the regression level
    print("\nStep 4: Calculate the regression level...")
    return_periods = [2, 5, 10, 25, 50, 100, 200, 500]

    # Calculate the probability of the excess value
    n_years = (rainfall.index[-1] - rainfall.index[0]).days / 365.25
    rate = len(extremes) / n_years  # The average number of events exceeding the threshold each year

    return_levels = {}
    for rp in return_periods:
        # Calculate the regression level using the GPD distribution
        probability = 1 / (rp * rate)

        # Using scipy for calculation, handling different parameter scenarios
        if len(params) == 3:
            level = stats.genpareto.isf(probability, *params) + threshold
        else:
            # Use the first two parameters. Set the position parameter to 0.
            level = stats.genpareto.isf(probability, params[0], 0, params[1]) + threshold

        return_levels[rp] = level

    # Create the result DataFrame
    results_df = pd.DataFrame.from_dict(return_levels, orient='index', columns=['return_level'])
    results_df.index.name = 'return_period'

    print("\nRainfall corresponding to different regression periods:")
    print("Recovery period (years) | Rainfall (mm)")
    print("-" * 30)
    for rp, level in return_levels.items():
        print(f"{rp:>10} | {level:>9.1f}")

    # 8. Draw a regression level chart
    plt.figure(figsize=(10, 6))
    plt.semilogx(list(return_levels.keys()), list(return_levels.values()),
                 'bo-', linewidth=2, markersize=6, label='Return level')
    plt.xlabel('Return period (years)')
    plt.ylabel('Precipitation (mm)')
    plt.title('Regression level of rainfall based on POT analysis')
    plt.legend()
    plt.grid(True, which="both", ls="-", alpha=0.3)
    plt.savefig('return_level_plot.png', dpi=300, bbox_inches='tight')
    plt.show()

    # 9. Generate diagnostic chart
    print("\nStep5: Generate diagnostic chart...")
    plot_diagnostic(rainfall, extremes, threshold, params, threshold_quantile)

    # 10. Generate future rainfall scenarios
    print("\nstep6: Generate future rainfall scenarios...")
    n_scenarios = 1000
    future_scenarios = generate_future_scenarios(threshold, params, n_scenarios)

    plt.figure(figsize=(10, 6))
    plt.hist(future_scenarios, bins=50, density=True, alpha=0.7,
             edgecolor='black', label='Simulation scenario distribution')
    plt.axvline(threshold, color='red', linestyle='--',
                label=f'Threshold value ({threshold:.1f} mm)')
    plt.xlabel('Daily rainfall (mm)')
    plt.ylabel('Probability density')
    plt.title('Distribution of future extreme rainfall scenarios based on the POT model')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('future_scenarios_distribution.png', dpi=300, bbox_inches='tight')
    plt.show()

    print(f"\nGenerated {n_scenarios} future extreme rainfall scenarios")
    print(f"Median in simulated scenarios: {np.median(future_scenarios):.1f} mm")
    print(f"Maximum value of simulation scenario: {np.max(future_scenarios):.1f} mm")

    return model, return_levels, future_scenarios, threshold, params


def plot_diagnostic(rainfall, extremes, threshold, params, threshold_quantile):
    """
    Generate the POT diagnostic chart
    """
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))

    # 1. Time series and extreme values
    axes[0, 0].plot(rainfall.index, rainfall.values, 'b-', alpha=0.5, linewidth=0.8, label='Original data')
    axes[0, 0].plot(extremes.index, extremes.values, 'ro', markersize=3,
                    label=f'Threshold-exceeding data (>{threshold:.1f} mm)')
    axes[0, 0].axhline(threshold, color='red', linestyle='--', alpha=0.7, label=f'Threshold')
    axes[0, 0].set_ylabel('Precipitation (mm)')
    axes[0, 0].set_title('Time series and extreme events')
    axes[0, 0].legend()
    axes[0, 0].grid(True, alpha=0.3)

    # 2. QQ plot
    excesses = extremes.values - threshold

    # Handle situations with different numbers of parameters
    if len(params) == 3:
        shape, loc, scale = params
    else:
        shape, scale = params[0], params[-1]
        loc = 0

    theoretical_quantiles = stats.genpareto.ppf(np.linspace(0.01, 0.99, len(excesses)), shape, loc, scale)
    axes[0, 1].plot(theoretical_quantiles, np.sort(excesses), 'bo', alpha=0.6)
    min_val = min(theoretical_quantiles.min(), excesses.min())
    max_val = max(theoretical_quantiles.max(), excesses.max())
    axes[0, 1].plot([min_val, max_val], [min_val, max_val], 'r--', linewidth=2)
    axes[0, 1].set_xlabel('Theoretical quantile')
    axes[0, 1].set_ylabel('Sample fractiles')
    axes[0, 1].set_title('QQ plot')
    axes[0, 1].grid(True, alpha=0.3)

    # 3. Excess mean plot (used for threshold selection verification)
    thresholds = np.percentile(rainfall, np.arange(90, 99, 1))
    mean_excess = []
    for t in thresholds:
        excess = rainfall[rainfall > t] - t
        mean_excess.append(excess.mean())

    axes[1, 0].plot(thresholds, mean_excess, 'bo-')
    axes[1, 0].axvline(threshold, color='red', linestyle='--', label=f'Selection threshold ({threshold_quantile * 100}%)')
    axes[1, 0].set_xlabel('Threshold (mm)')
    axes[1, 0].set_ylabel('Average excess(mm)')
    axes[1, 0].set_title('Average Excess Plot')
    axes[1, 0].legend()
    axes[1, 0].grid(True, alpha=0.3)

    # 4. probability density function
    x = np.linspace(0, extremes.max() - threshold, 100)
    pdf = stats.genpareto.pdf(x, shape, loc, scale)
    axes[1, 1].plot(x, pdf, 'b-', linewidth=2, label='GPD fitting')
    axes[1, 1].hist(excesses, bins=20, density=True,
                    alpha=0.5, edgecolor='black', label='Excess data')
    axes[1, 1].set_xlabel('Excessive rainfall (mm)')
    axes[1, 1].set_ylabel('Probability density')
    axes[1, 1].set_title('Generalized Pareto Distribution Fitting')
    axes[1, 1].legend()
    axes[1, 1].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig('pot_diagnostic_plots.png', dpi=300, bbox_inches='tight')
    plt.show()


def generate_future_scenarios(threshold, params, n_scenarios=1000):
    """
    Generate future extreme rainfall scenarios based on the POT model
    """
    if len(params) == 3:
        shape, loc, scale = params
    else:
        shape, scale = params[0], params[-1]
        loc = 0

    # Sampling the excess values from the fitted GPD distribution
    excess_values = stats.genpareto.rvs(shape, loc, scale, size=n_scenarios)
    # Adding the threshold value results in the complete rainfall amount.
    rainfall_scenarios = excess_values + threshold
    return rainfall_scenarios


# Carry out a complete POT analysis
if __name__ == "__main__":
    try:
        # Carry out a complete POT analysis
        model, return_levels, future_scenarios, threshold, params = comprehensive_pot_analysis(
            "",
            threshold_quantile=0.99
        )

        results_summary = {
            'threshold': threshold,
            'n_extremes': len(future_scenarios),
            'gpd_params': params,
            'return_levels': return_levels,
            'future_scenarios_median': np.median(future_scenarios),
            'future_scenarios_max': np.max(future_scenarios)
        }

        print(f"\nAnalysis completed! Key results:")
        print(f"  Threshold: {threshold:.1f} mm")
        print(f"  GPD parameters: {params}")
        print(f"  Median scenario for the future: {np.median(future_scenarios):.1f} mm")
        print(f"  Maximum scenario for the future: {np.max(future_scenarios):.1f} mm")

        print("\nRegression to the mean result:")
        for rp, level in return_levels.items():
            print(f"  {rp}return year: {level:.1f} mm")

        results_df = pd.DataFrame.from_dict(return_levels, orient='index', columns=['Rainfall_mm'])
        results_df.index.name = 'Return_Period_Years'
        results_df.to_csv('pot_return_levels.csv')
        print("\nThe regression level results have been saved to 'pot_return_levels.csv'")

    except Exception as e:
        print(f"Errors occurred during the analysis process.: {e}")
        import traceback

        traceback.print_exc()