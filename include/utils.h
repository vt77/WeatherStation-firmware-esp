
/**
 *  This function converts weather station pressure to sea level pressure 
 *   
 *  @param pressure_hpa current weather station pressure 
 *  @param alt_hpa normal pressure according to weather station altitude (about plus 1 mmHHg per 12 meter )
 */

float to_mmHg_at_sealevel(float pressure_hpa, float alt_hpa)
{
    return pressure_hpa * 0.75 * (float)(760.0 / alt_hpa);
}

