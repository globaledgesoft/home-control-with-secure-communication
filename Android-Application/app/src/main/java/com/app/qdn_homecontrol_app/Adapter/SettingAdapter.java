package com.app.qdn_homecontrol_app.Adapter;

import android.app.Activity;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.support.design.widget.Snackbar;
import com.app.qdn_homecontrol_app.R;
import java.util.ArrayList;

public class SettingAdapter extends RecyclerView.Adapter<SettingAdapter.SettingsViewHolder> {
    private TextView settingsItem;
    private ArrayList<String> settingsList;
    private Activity mActivity;

    public interface ItemClickListener {
        void onItemClick (int position, View view);
    }

    public SettingAdapter(ArrayList<String> settingsList, Activity activity) {
        this.settingsList = settingsList;
        this.mActivity = activity;
    }


    @NonNull
    @Override
    public SettingsViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View itemView = LayoutInflater.from(viewGroup.getContext())
                .inflate(R.layout.settings_row, viewGroup, false);

        return new SettingsViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(@NonNull SettingsViewHolder settingsViewHolder, int i) {
        settingsItem.setText(settingsList.get(i));

        settingsViewHolder.setClicklistner(new ItemClickListener() {
            @Override
            public void onItemClick(int position, View view) {
                Snackbar.make(view, settingsList.get(position)+" activity is not yet implemented", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });
    }

    @Override
    public int getItemCount() {
        return settingsList.size();
    }

    public class SettingsViewHolder extends RecyclerView.ViewHolder implements
    View.OnClickListener{

        private ItemClickListener clickListner;
        public SettingsViewHolder(@NonNull View itemView) {
            super(itemView);
            settingsItem = itemView.findViewById(R.id.settingTextView);
            itemView.setOnClickListener(this);
        }

        public void setClicklistner(ItemClickListener clickListner) {
            this.clickListner = clickListner;
        }

        @Override
        public void onClick(View v) {
            clickListner.onItemClick(getAdapterPosition(), v);

        }
    }
}
