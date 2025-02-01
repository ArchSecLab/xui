from scipy.stats import norm
import math
def NormalCDF(x):
    return norm.cdf(x)
def find_next(packet_type, min_value, stddev):
    # packet value is the expected value of the functions so we need to readjust it to the mean
    # packet_type = mean + stddev*(pdf((min_value-mean)/stddev))/(1 -phi_alpha)
    mean = packet_type
    alpha = (min_value - mean) / stddev
    b= mean+min_value
    beta = (b - mean)/stddev
    phi_alpha = NormalCDF(alpha)
    pdf_alpha = norm.pdf(alpha)
    phi_beta = NormalCDF(beta)
    pdf_beta = norm.pdf(beta)
    err = stddev * (pdf_alpha-pdf_beta)/(phi_beta - phi_alpha)
    print(err,mean+err)
    while abs(packet_type - mean - err) > 0.00000001:
        mean = (mean + packet_type - err)/2
        if mean < min_value:
            mean = min_value
            break
        alpha = (min_value - mean) / stddev
        beta = (b - mean)/stddev
        phi_alpha = NormalCDF(alpha)
        pdf_alpha = norm.pdf(alpha)
        phi_beta = NormalCDF(beta)
        pdf_beta = norm.pdf(beta)
        err = stddev * (pdf_alpha-pdf_beta)/(phi_beta - phi_alpha)
    alpha = (min_value - mean) / stddev
    beta = (b - mean)/stddev
    phi_alpha = NormalCDF(alpha)
    pdf_alpha = norm.pdf(alpha)
    phi_beta = NormalCDF(beta)
    pdf_beta = norm.pdf(beta)
    return mean, mean + stddev * (pdf_alpha-pdf_beta)/(phi_beta - phi_alpha)
print(find_next(20, 10, 30))
